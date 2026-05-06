#include "huffman.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage : %s <fichier_code>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* ------------------------------------------------------------------
     * 1. Lecture du fichier de codage
     * ------------------------------------------------------------------ */
    FILE *f = fopen(argv[1], "rb");
    if (f == NULL) {
        fprintf(stderr, "Erreur : impossible d'ouvrir %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    /* Lire tout le contenu dans un tampon dynamique */
    fseek(f, 0, SEEK_END);
    long taille = ftell(f);
    rewind(f);

    char *bits = (char *)malloc((size_t)(taille + 1));
    if (bits == NULL) {
        fprintf(stderr, "Erreur : allocation memoire\n");
        fclose(f);
        return EXIT_FAILURE;
    }

    /* Lire caractere par caractere en ignorant les sauts de ligne */
    long nb_bits = 0;
    int c;
    while ((c = fgetc(f)) != EOF) {
        if (c == '0' || c == '1') {
            bits[nb_bits++] = (char)c;
        }
    }
    bits[nb_bits] = '\0';
    fclose(f);

    if (nb_bits == 0) {
        fprintf(stderr, "Erreur : fichier de codage vide ou invalide\n");
        free(bits);
        return EXIT_FAILURE;
    }

    /* ------------------------------------------------------------------
     * 2. Reconstruction de l'arbre depuis l'en-tete
     * ------------------------------------------------------------------ */
    int pos = 0;
    PArbre arbre = decoder_arbre(bits, &pos);

    /* ------------------------------------------------------------------
     * 3. Decodage du texte
     * Parcourir l'arbre bit par bit depuis pos jusqu'a la fin du flux.
     * Quand on atteint une feuille, ecrire le caractere et repartir de la racine.
     * ------------------------------------------------------------------ */
    PArbre courant = arbre;

    /* Cas degenere : arbre a un seul noeud (un seul caractere distinct) */
    if (courant->valeur.est_feuille) {
        while (pos < nb_bits) {
            putchar((int)courant->valeur.caractere);
            pos++;
        }
        LibererArbre(arbre);
        free(bits);
        return EXIT_SUCCESS;
    }

    while (pos < nb_bits) {
        if (bits[pos] == '0') {
            courant = Gauche(courant);
        } else {
            courant = Droit(courant);
        }
        pos++;

        if (courant == NULL) {
            fprintf(stderr, "Erreur : fichier de codage corrompu\n");
            LibererArbre(arbre);
            free(bits);
            return EXIT_FAILURE;
        }

        if (courant->valeur.est_feuille) {
            putchar((int)courant->valeur.caractere);
            courant = arbre; /* repartir de la racine */
        }
    }

    LibererArbre(arbre);
    free(bits);
    return EXIT_SUCCESS;
}
