#include "huffman.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage : %s <fichier>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* ------------------------------------------------------------------
     * 1. Lecture du fichier et comptage des frequences
     * ------------------------------------------------------------------ */
    FILE *f = fopen(argv[1], "rb");
    if (f == NULL) {
        fprintf(stderr, "Erreur : impossible d'ouvrir %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    int frequences[MAX_CHAR] = {0};
    long taille_originale = 0;
    int octet;

    while ((octet = fgetc(f)) != EOF) {
        frequences[(unsigned char)octet]++;
        taille_originale++;
    }
    rewind(f);

    if (taille_originale == 0) {
        fprintf(stderr, "Erreur : fichier vide\n");
        fclose(f);
        return EXIT_FAILURE;
    }

    /* ------------------------------------------------------------------
     * 2. Construction de l'arbre de Huffman
     * ------------------------------------------------------------------ */
    PArbre arbre = construire_arbre(frequences);

    /* ------------------------------------------------------------------
     * 3. Construction de la table des codes prefixes
     * ------------------------------------------------------------------ */
    Code codes[MAX_CHAR];
    for (int i = 0; i < MAX_CHAR; i++) {
        codes[i].longueur = 0;
        codes[i].bits[0]  = '\0';
    }

    char chemin[MAX_CODE];
    construire_codes(arbre, codes, chemin, 0);

    /* ------------------------------------------------------------------
     * 4. Encodage de l'arbre vers stdout
     * ------------------------------------------------------------------ */
    /* Taille maximale de l'en-tete : chaque feuille = 1 + 8 bits,
     * chaque noeud interne = 1 bit. Pour 256 feuilles : (1+8)*256 + 255 = 2559 bits */
    char tampon_arbre[4096];
    int  pos_arbre = 0;
    encoder_arbre(arbre, tampon_arbre, &pos_arbre);
    tampon_arbre[pos_arbre] = '\0';
    printf("%s", tampon_arbre);

    /* ------------------------------------------------------------------
     * 5. Encodage du texte vers stdout
     * ------------------------------------------------------------------ */
    long bits_texte = 0;

    while ((octet = fgetc(f)) != EOF) {
        unsigned char c = (unsigned char)octet;
        printf("%s", codes[c].bits);
        bits_texte += codes[c].longueur;
    }
    printf("\n");

    fclose(f);

    /* ------------------------------------------------------------------
     * 6. Calcul et affichage du taux de compression sur stderr
     * ------------------------------------------------------------------ */
    long bits_total     = pos_arbre + bits_texte;
    long bits_originaux = taille_originale * 8;
    double taux = 100.0 * (double)(bits_originaux - bits_total) / (double)bits_originaux;

    fprintf(stderr, "Taux de compression : %.1f %%\n", taux);

    LibererArbre(arbre);
    return EXIT_SUCCESS;
}
