#include "huffman.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* ===================================================================
 * Arbre binaire
 * =================================================================== */

PArbre ArbreVide(void) {
    return NULL;
}

bool EstArbreVide(PArbre a) {
    return a == NULL;
}

element Racine(PArbre a) {
    if (EstArbreVide(a)) {
        fprintf(stderr, "Erreur : Racine d'un arbre vide\n");
        exit(EXIT_FAILURE);
    }
    return a->valeur;
}

PArbre Gauche(PArbre a) {
    if (EstArbreVide(a)) return NULL;
    return a->gauche;
}

PArbre Droit(PArbre a) {
    if (EstArbreVide(a)) return NULL;
    return a->droite;
}

PArbre Construire(element e, PArbre g, PArbre d) {
    PArbre n = (PArbre)malloc(sizeof(struct noeud));
    if (n == NULL) {
        fprintf(stderr, "Erreur : allocation memoire\n");
        exit(EXIT_FAILURE);
    }
    n->valeur = e;
    n->gauche = g;
    n->droite = d;
    return n;
}

void LibererArbre(PArbre a) {
    if (!EstArbreVide(a)) {
        LibererArbre(a->gauche);
        LibererArbre(a->droite);
        free(a);
    }
}

/* ===================================================================
 * Liste chainee d'arbres triee par poids croissant
 * =================================================================== */

PListe ListeVide(void) {
    return NULL;
}

bool EstListeVide(PListe l) {
    return l == NULL;
}

/* Insertion triee par poids croissant (pour la liste des feuilles) */
PListe Inserer(PArbre a, PListe l) {
    struct cellule *nouvelle = (struct cellule *)malloc(sizeof(struct cellule));
    if (nouvelle == NULL) {
        fprintf(stderr, "Erreur : allocation memoire liste\n");
        exit(EXIT_FAILURE);
    }
    nouvelle->arbre = a;

    /* Cas : liste vide ou poids <= tete */
    if (EstListeVide(l) || a->valeur.poids <= l->arbre->valeur.poids) {
        nouvelle->suivant = l;
        return nouvelle;
    }

    /* Chercher la position d'insertion */
    struct cellule *courant = l;
    while (courant->suivant != NULL &&
           courant->suivant->arbre->valeur.poids < a->valeur.poids) {
        courant = courant->suivant;
    }
    nouvelle->suivant = courant->suivant;
    courant->suivant = nouvelle;
    return l;
}

PArbre TeteArbre(PListe l) {
    if (EstListeVide(l)) {
        fprintf(stderr, "Erreur : TeteArbre d'une liste vide\n");
        exit(EXIT_FAILURE);
    }
    return l->arbre;
}

PListe Queue(PListe l) {
    if (EstListeVide(l)) return NULL;
    PListe suite = l->suivant;
    free(l);
    return suite;
}

/* Ajout en fin de liste en O(n) — la liste des noeuds reste courte */
PListe AjouterFin(PArbre a, PListe l) {
    struct cellule *nouvelle = (struct cellule *)malloc(sizeof(struct cellule));
    if (nouvelle == NULL) {
        fprintf(stderr, "Erreur : allocation memoire liste\n");
        exit(EXIT_FAILURE);
    }
    nouvelle->arbre   = a;
    nouvelle->suivant = NULL;

    if (EstListeVide(l)) return nouvelle;

    struct cellule *courant = l;
    while (courant->suivant != NULL) courant = courant->suivant;
    courant->suivant = nouvelle;
    return l;
}

void LibererListe(PListe l) {
    while (!EstListeVide(l)) {
        PListe suite = l->suivant;
        free(l);
        l = suite;
    }
}

/* ===================================================================
 * Selection du minimum parmi deux listes
 * Retourne l'arbre de poids minimal en tete de l_feuilles ou l_noeuds
 * et avance la liste correspondante.
 * =================================================================== */
static PArbre extraire_min(PListe *l_feuilles, PListe *l_noeuds) {
    bool f_vide = EstListeVide(*l_feuilles);
    bool n_vide = EstListeVide(*l_noeuds);

    if (f_vide && n_vide) {
        fprintf(stderr, "Erreur : extraire_min sur deux listes vides\n");
        exit(EXIT_FAILURE);
    }

    if (f_vide) {
        PArbre a = TeteArbre(*l_noeuds);
        *l_noeuds = Queue(*l_noeuds);
        return a;
    }
    if (n_vide) {
        PArbre a = TeteArbre(*l_feuilles);
        *l_feuilles = Queue(*l_feuilles);
        return a;
    }

    /* Les deux listes sont non vides : comparer les tetes */
    if ((*l_feuilles)->arbre->valeur.poids <= (*l_noeuds)->arbre->valeur.poids) {
        PArbre a = TeteArbre(*l_feuilles);
        *l_feuilles = Queue(*l_feuilles);
        return a;
    } else {
        PArbre a = TeteArbre(*l_noeuds);
        *l_noeuds = Queue(*l_noeuds);
        return a;
    }
}

/* ===================================================================
 * Construction de l'arbre de Huffman
 * =================================================================== */
PArbre construire_arbre(int frequences[MAX_CHAR]) {
    /* 1. Construire la liste triee des feuilles */
    PListe l_feuilles = ListeVide();

    for (int i = 0; i < MAX_CHAR; i++) {
        if (frequences[i] > 0) {
            element e;
            e.caractere   = (unsigned char)i;
            e.poids        = frequences[i];
            e.est_feuille  = true;
            PArbre feuille = Construire(e, ArbreVide(), ArbreVide());
            l_feuilles     = Inserer(feuille, l_feuilles);
        }
    }

    /* Cas degenere : un seul caractere distinct */
    if (!EstListeVide(l_feuilles) && EstListeVide(l_feuilles->suivant)) {
        PArbre a = TeteArbre(l_feuilles);
        free(l_feuilles);
        return a;
    }

    /* 2. Liste des noeuds internes (initialement vide) */
    PListe l_noeuds = ListeVide();

    /* 3. Fusionner jusqu'a obtenir un seul arbre */
    while (true) {
        /* Compter le nombre d'arbres restants */
        int nb = 0;
        for (struct cellule *c = l_feuilles; c != NULL; c = c->suivant) nb++;
        for (struct cellule *c = l_noeuds;   c != NULL; c = c->suivant) nb++;
        if (nb == 1) break;

        /* Extraire les deux arbres de poids minimum */
        PArbre a1 = extraire_min(&l_feuilles, &l_noeuds);
        PArbre a2 = extraire_min(&l_feuilles, &l_noeuds);

        /* Creer un nouveau noeud interne */
        element e;
        e.caractere  = 0;
        e.poids      = a1->valeur.poids + a2->valeur.poids;
        e.est_feuille = false;
        PArbre nouveau = Construire(e, a1, a2);

        /* Ajouter a la fin de la liste des noeuds */
        l_noeuds = AjouterFin(nouveau, l_noeuds);
    }

    /* L'arbre final est dans la liste restante */
    if (!EstListeVide(l_noeuds))  return TeteArbre(l_noeuds);
    if (!EstListeVide(l_feuilles)) return TeteArbre(l_feuilles);

    return ArbreVide();
}

/* ===================================================================
 * Construction de la table des codes par parcours prefixe
 * =================================================================== */
void construire_codes(PArbre a, Code codes[MAX_CHAR],
                      char chemin[], int profondeur) {
    if (EstArbreVide(a)) return;

    if (a->valeur.est_feuille) {
        /* Feuille : enregistrer le code */
        int c = (int)a->valeur.caractere;
        chemin[profondeur] = '\0';
        memcpy(codes[c].bits, chemin, (size_t)(profondeur + 1));
        codes[c].longueur = profondeur;
        return;
    }

    /* Aller a gauche : ajouter '0' */
    chemin[profondeur] = '0';
    construire_codes(a->gauche, codes, chemin, profondeur + 1);

    /* Aller a droite : ajouter '1' */
    chemin[profondeur] = '1';
    construire_codes(a->droite, codes, chemin, profondeur + 1);
}

/* ===================================================================
 * Encodage de l'arbre (parcours prefixe)
 * 0 pour noeud interne, 1 + 8 bits ASCII pour feuille
 * =================================================================== */
void encoder_arbre(PArbre a, char *tampon, int *pos) {
    if (EstArbreVide(a)) return;

    if (a->valeur.est_feuille) {
        tampon[(*pos)++] = '1';
        /* Ecrire les 8 bits ASCII du caractere, MSB en premier */
        unsigned char c = a->valeur.caractere;
        for (int i = 7; i >= 0; i--) {
            tampon[(*pos)++] = ((c >> i) & 1) ? '1' : '0';
        }
    } else {
        tampon[(*pos)++] = '0';
        encoder_arbre(a->gauche, tampon, pos);
        encoder_arbre(a->droite, tampon, pos);
    }
}

/* ===================================================================
 * Decodage de l'arbre depuis un flux de bits
 * =================================================================== */
PArbre decoder_arbre(const char *bits, int *pos) {
    if (bits[*pos] == '0') {
        (*pos)++;
        /* Noeud interne */
        PArbre gauche = decoder_arbre(bits, pos);
        PArbre droite = decoder_arbre(bits, pos);
        element e;
        e.caractere  = 0;
        e.poids      = 0;
        e.est_feuille = false;
        return Construire(e, gauche, droite);
    } else {
        (*pos)++;
        /* Feuille : lire 8 bits ASCII */
        unsigned char c = 0;
        for (int i = 7; i >= 0; i--) {
            if (bits[*pos] == '1') c = (unsigned char)(c | (1 << i));
            (*pos)++;
        }
        element e;
        e.caractere  = c;
        e.poids      = 0;
        e.est_feuille = true;
        return Construire(e, ArbreVide(), ArbreVide());
    }
}
