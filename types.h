/* types.h - structures partagees et constantes de ShopFlow
 * (Date, Adresse, Produit, LigneCommande, Commande, Client, Boutique).
 * Inclus par tous les modules pour eviter les inclusions circulaires. */
#ifndef TYPES_H
#define TYPES_H

/* Tailles des chaines (taille du tableau, '\0' compris) */
#define TAILLE_REFERENCE    12
#define TAILLE_DESIGNATION  60
#define TAILLE_CATEGORIE    30
#define TAILLE_NOM          50
#define TAILLE_EMAIL        80
#define TAILLE_TELEPHONE    15
#define TAILLE_RUE          80
#define TAILLE_VILLE        50
#define TAILLE_CODE_POSTAL   6
#define TAILLE_PAYS         50

#define CAPACITE_INITIALE    4   /* premiere allocation d'un tableau dynamique */

typedef struct {
    char rue[TAILLE_RUE];
    char ville[TAILLE_VILLE];
    char codePostal[TAILLE_CODE_POSTAL];
    char pays[TAILLE_PAYS];
} Adresse;

typedef struct {
    char  reference[TAILLE_REFERENCE];
    char  designation[TAILLE_DESIGNATION];
    char  categorie[TAILLE_CATEGORIE];
    float prixUnitaireHT;
    int   quantiteStock;
    int   seuilAlerte;
    int   actif;                 /* 1 = actif, 0 = supprime (logique) */
} Produit;

/* La structure Commande sera completee dans la suite du projet. */
typedef struct Commande Commande;

typedef struct {
    int       idClient;
    char      nom[TAILLE_NOM];
    char      prenom[TAILLE_NOM];
    char      email[TAILLE_EMAIL];
    char      telephone[TAILLE_TELEPHONE];
    Adresse   adresse;
    int       pointsFidelite;
    int       nbCommandes;
    int       capaciteCommandes;
    Commande *commandes;         /* NULL tant qu'aucune commande */
    int       actif;
} Client;

typedef struct {
    char     nom[TAILLE_NOM];
    Produit *produits;
    int      nbProduits;
    int      capaciteProduits;
    Client  *clients;
    int      nbClients;
    int      capaciteClients;
    int      numeroSuivant;      /* prochain numero de commande */
    int      idClientSuivant;    /* prochain identifiant client */
} Boutique;

#endif /* TYPES_H */
