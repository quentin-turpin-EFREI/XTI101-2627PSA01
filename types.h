/* types.h - structures partagees et constantes de ShopFlow. */
#ifndef TYPES_H
#define TYPES_H

/* Tailles des chaines, caractere nul compris. */
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

#define CAPACITE_INITIALE    4

typedef enum {
    OK = 0,
    ERR_MEMOIRE,
    ERR_DOUBLON,
    ERR_INTROUVABLE,
    ERR_INVALIDE,
    ERR_INTERDIT
} CodeRetour;

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
    int   actif;
} Produit;

typedef enum {
    EN_COURS = 0,
    EXPEDIEE,
    LIVREE,
    ANNULEE
} StatutCommande;

typedef struct {
    int jour;
    int mois;
    int annee;
} Date;

typedef struct {
    char refProduit[TAILLE_REFERENCE];
    char designation[TAILLE_DESIGNATION];
    float prixUnitaireHT;
    int quantite;
    float sousTotalHT;
} LigneCommande;

typedef struct Commande {
    int numero;
    int idClient;
    Date dateCommande;
    StatutCommande statut;
    int nbLignes;
    int capaciteLignes;
    LigneCommande *lignes;
    Adresse adresseLivraison;
    float montantBrutHT;
    float tauxRemise;
    float montantRemise;
    float montantHT;
    float montantTVA;
    float fraisPort;
    float montantTTC;
} Commande;

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
    Commande *commandes;
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
    int      numeroSuivant;
    int      idClientSuivant;
} Boutique;

#endif /* TYPES_H */
