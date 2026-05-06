# Compression de Huffman — ReadMe

## Auteurs

Master BIMS 2e année — Université de Rouen Normandie — 2025/2026

## Compilation

```bash
make        # compile hcode et unhcode
make clean  # supprime les fichiers objets et les binaires
```

Options gcc imposées : `-std=c2x -Wall -Wconversion -Werror -Wextra
-Wfatal-errors -Wpedantic -Wwrite-strings -O2`

## Utilisation

```bash
./hcode monTexte.txt > monTexte.huf   # compression (taux sur stderr)
./unhcode monTexte.huf                # décompression vers stdout
```

---

## Structures de données

### Arbre binaire (`huffman.h / huffman.c`)

On réutilise l'implémentation vue en cours avec le type `PArbre`
(pointeur vers `struct noeud`). Chaque nœud stocke un `ElementHuffman` :

```c
typedef struct {
    unsigned char caractere;  /* valide uniquement pour les feuilles */
    int           poids;      /* fréquence ou somme des fréquences   */
    bool          est_feuille;
} ElementHuffman;
```

Les primitives utilisées sont `Construire`, `Gauche`, `Droit`, `Racine`,
`EstArbreVide`, `LibererArbre`.

### Listes chaînées d'arbres (`huffman.c`)

Conformément au sujet, on maintient **deux listes** :

| Liste | Contenu | Ordre |
|-------|---------|-------|
| `l_feuilles` | feuilles initiales | trié par poids croissant (insertion triée) |
| `l_noeuds`   | nœuds internes créés | trié par poids croissant (ajout en fin) |

La sélection des deux arbres de poids minimum se réduit à comparer les
têtes des deux listes (`extraire_min`), soit **O(1)** par extraction.
L'ajout du nouveau nœud interne en fin de `l_noeuds` est **O(n)** sur
la liste des nœuds, mais celle-ci reste toujours plus courte que le
nombre de caractères distincts (≤ 255 éléments) : le coût global
de la construction est **O(n log n)** avec n = nombre de caractères
distincts.

---

## Algorithmes

### Calcul des fréquences (`hcode.c`)

Parcours linéaire du fichier avec un tableau `int frequences[256]`.
Complexité : **O(|texte|)**.

### Construction de l'arbre de Huffman (`construire_arbre`)

1. Créer une feuille par caractère distinct et les insérer dans
   `l_feuilles` triée.
2. Répéter jusqu'à n'avoir plus qu'un seul arbre :
   - Extraire les deux arbres de poids minimum (`extraire_min`).
   - Créer un nœud interne dont le poids = somme des deux.
   - L'ajouter **en fin** de `l_noeuds` (la liste des nœuds reste triée
     car les poids sont croissants).

### Génération des codes préfixes (`construire_codes`)

Parcours préfixe récursif de l'arbre : aller à gauche ajoute `'0'` au
chemin courant, aller à droite ajoute `'1'`. À chaque feuille, le chemin
courant est copié dans la table `codes[caractere]`.

### Encodage de l'arbre (`encoder_arbre`)

Parcours préfixe :
- Nœud interne → écrire `0`.
- Feuille → écrire `1` puis les **8 bits ASCII** du caractère (MSB en
  premier).

Taille de l'en-tête pour *k* caractères distincts :
`k × 9 + (k−1)` bits.

### Encodage du texte (`hcode.c`)

Relecture du fichier source ; chaque octet est remplacé par son code
de la table. La sortie est une chaîne de `'0'` et `'1'` au format
caractère ASCII.

### Décodage de l'arbre (`decoder_arbre`)

Reconstruction récursive depuis le flux de bits en suivant la même
convention que l'encodage.

### Décodage du texte (`unhcode.c`)

Parcours du flux de bits en descendant l'arbre depuis la racine.
Quand une feuille est atteinte, le caractère est écrit sur stdout et
on repart de la racine.

---

## Justification des choix

- **Deux listes plutôt qu'un tas (heap)** : exigé par le sujet pour
  garantir la sélection en O(1). L'invariant de tri est maintenu
  naturellement car les poids des nœuds internes sont toujours ≥ au
  dernier nœud ajouté.
- **Format de sortie texte (`'0'`/`'1'`)** : plus simple à déboguer et
  conforme aux exemples du sujet. En production on utiliserait un
  encodage binaire pour doubler le taux de compression.
- **Table `codes[256]`** : accès en O(1) pendant l'encodage du texte.

---

## Tests

| Fichier | Contenu | Taux de compression |
|---------|---------|-------------------|
| `tests/monTexte.txt` | `abbraccaddabrra` (15 chars) | 30.8 % |
| `tests/long.txt` | 200 chars aléatoires | ~24 % |
| `tests/ascii.txt` | 95 caractères ASCII distincts | négatif (normal) |

Le taux négatif sur `ascii.txt` est attendu : quand tous les caractères
ont la même fréquence, les codes Huffman ont tous la même longueur et
l'en-tête alourdit le fichier compressé.

La correction du décodage est vérifiée par comparaison octet par octet
avec le fichier source original.
