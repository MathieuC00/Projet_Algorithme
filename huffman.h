#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdbool.h>
#include <stddef.h>

/* -----------------------------------------------------------------------
 * Type element pour l'arbre de Huffman
 * Un noeud interne a caractere = 0 et est_feuille = false
 * Une feuille a le caractere original et est_feuille = true
 * ----------------------------------------------------------------------- */
typedef struct {
    unsigned char caractere; /* caractere original (valide seulement si feuille) */
    int           poids;     /* frequence / somme des frequences                */
    bool          est_feuille;
} ElementHuffman;

/* -----------------------------------------------------------------------
 * Arbre binaire generique (adapte depuis le cours)
 * On stocke ElementHuffman dans chaque noeud.
 * ----------------------------------------------------------------------- */
typedef ElementHuffman element;

typedef struct noeud {
    element       valeur;
    struct noeud *gauche;
    struct noeud *droite;
} *PArbre;

/* Primitives de l'arbre binaire */
PArbre ArbreVide(void);
bool   EstArbreVide(PArbre a);
element Racine(PArbre a);
PArbre Gauche(PArbre a);
PArbre Droit(PArbre a);
PArbre Construire(element e, PArbre g, PArbre d);
void   LibererArbre(PArbre a);

/* -----------------------------------------------------------------------
 * Liste chainee d'arbres (files de priorite par tri croissant du poids)
 * Utilisee pour les feuilles et les noeuds internes.
 * ----------------------------------------------------------------------- */
typedef struct cellule {
    PArbre        arbre;
    struct cellule *suivant;
} *PListe;

PListe   ListeVide(void);
bool     EstListeVide(PListe l);
PListe   Inserer(PArbre a, PListe l);   /* insertion triee par poids croissant */
PArbre   TeteArbre(PListe l);           /* arbre en tete                        */
PListe   Queue(PListe l);               /* liste sans la tete                   */
PListe   AjouterFin(PArbre a, PListe l);/* ajout en O(1) a la fin              */
void     LibererListe(PListe l);

/* -----------------------------------------------------------------------
 * Codes de Huffman (table des codes prefixes)
 * ----------------------------------------------------------------------- */
#define MAX_CHAR 256
#define MAX_CODE 256   /* longueur max d'un code en bits */

typedef struct {
    char  bits[MAX_CODE]; /* chaine de '0' et '1' */
    int   longueur;
} Code;

/* -----------------------------------------------------------------------
 * Fonctions principales
 * ----------------------------------------------------------------------- */

/* Construction de l'arbre depuis un tableau de frequences */
PArbre construire_arbre(int frequences[MAX_CHAR]);

/* Remplissage de la table des codes par parcours prefixe */
void construire_codes(PArbre a, Code codes[MAX_CHAR],
                      char chemin[], int profondeur);

/* Encodage de l'arbre (parcours prefixe) vers stdout bit par bit */
void encoder_arbre(PArbre a, char *tampon, int *pos);

/* Decodage de l'arbre depuis un flux de bits */
PArbre decoder_arbre(const char *bits, int *pos);

#endif /* HUFFMAN_H */
