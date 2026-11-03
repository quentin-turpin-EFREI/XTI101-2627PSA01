/* initialisation.c - jeu complet de donnees de demonstration. */
#include <stdlib.h>
#include <string.h>

#include "initialisation.h"
#include "client.h"
#include "commande.h"
#include "produit.h"

typedef struct {
    const char *nom;
    const char *prenom;
    const char *email;
    const char *telephone;
    const char *rue;
    const char *ville;
    const char *codePostal;
    const char *pays;
} ClientDemo;

typedef struct {
    int idClient;
    StatutCommande statut;
    const LigneCommande *lignes;
    int nbLignes;
} CommandeDemo;

static CodeRetour ajouterClientDemo(Boutique *boutique,
                                    const ClientDemo *demo)
{
    Client client = {0};
    int idAttribue;

    strcpy(client.nom, demo->nom);
    strcpy(client.prenom, demo->prenom);
    strcpy(client.email, demo->email);
    strcpy(client.telephone, demo->telephone);
    strcpy(client.adresse.rue, demo->rue);
    strcpy(client.adresse.ville, demo->ville);
    strcpy(client.adresse.codePostal, demo->codePostal);
    strcpy(client.adresse.pays, demo->pays);

    return ajouterClient(boutique, &client, &idAttribue);
}

static CodeRetour ajouterCommandeDemo(Boutique *boutique,
                                      const CommandeDemo *demo)
{
    int numero;
    int indiceClient;
    Client *client;
    Commande *commande;
    int i;
    CodeRetour resultat;

    if (demo == NULL || demo->lignes == NULL || demo->nbLignes <= 0)
        return ERR_INVALIDE;

    resultat = creerCommande(boutique, demo->idClient, &numero);
    if (resultat != OK)
        return resultat;

    indiceClient = trouverClient(boutique, demo->idClient);
    if (indiceClient < 0)
        return ERR_INVALIDE;

    client = &boutique->clients[indiceClient];
    commande = &client->commandes[client->nbCommandes - 1];
    commande->lignes = malloc((size_t)demo->nbLignes
                              * sizeof *commande->lignes);
    if (commande->lignes == NULL)
        return ERR_MEMOIRE;
    commande->nbLignes = demo->nbLignes;
    commande->capaciteLignes = demo->nbLignes;

    for (i = 0; i < demo->nbLignes; i++) {
        int indiceProduit = trouverProduit(boutique,
                                           demo->lignes[i].refProduit);

        if (indiceProduit < 0 || !boutique->produits[indiceProduit].actif
            || demo->lignes[i].quantite <= 0
            || (demo->statut != ANNULEE
                && boutique->produits[indiceProduit].quantiteStock
                    < demo->lignes[i].quantite))
            return ERR_INVALIDE;
        commande->lignes[i] = demo->lignes[i];
    }

    commande->statut = demo->statut;
    recalculerMontantsCommande(commande);
    if (demo->statut != ANNULEE) {
        for (i = 0; i < demo->nbLignes; i++) {
            int indiceProduit = trouverProduit(boutique,
                                               demo->lignes[i].refProduit);
            boutique->produits[indiceProduit].quantiteStock -=
                demo->lignes[i].quantite;
        }
        ajouterPointsFidelite(&boutique->clients[indiceClient],
                              commande->montantTTC);
    }
    return OK;
}

CodeRetour initialiserDonneesTest(Boutique *boutique)
{
    Boutique donnees = {0};
    static const Produit produitsDemo[NB_PRODUITS_DEMO] = {
        {"PRD-0001", "Clavier mecanique",  "Informatique",  79.90f,  42, 10, 1},
        {"PRD-0002", "Souris sans fil",    "Informatique",  24.50f,   8, 15, 1},
        {"PRD-0003", "Ecran 27 pouces",    "Informatique", 199.00f,   6,  5, 1},
        {"PRD-0004", "Casque audio",       "Audio",         89.50f,  25, 10, 1},
        {"PRD-0005", "Enceinte Bluetooth", "Audio",         59.90f,  12,  8, 1},
        {"PRD-0006", "Cable HDMI 2 m",     "Accessoires",   10.10f, 120, 30, 1},
        {"PRD-0007", "Hub USB-C 7 ports",  "Accessoires",   49.90f,   4, 10, 1},
        {"PRD-0008", "Disque SSD 1 To",    "Stockage",     109.00f,  18,  6, 1}
    };
    static const ClientDemo clientsDemo[NB_CLIENTS_DEMO] = {
        {"Martin", "Aline", "aline.martin@example.fr", "0611111111",
         "10 rue Victor Hugo", "Paris", "75001", "France"},
        {"Bernard", "Karim", "karim.bernard@example.fr", "0622222222",
         "24 avenue des Fleurs", "Lyon", "69001", "France"},
        {"Leroy", "Sophie", "sophie.leroy@example.fr", "0633333333",
         "8 place du Marche", "Nantes", "44000", "France"},
        {"Petit", "Julie", "julie.petit@example.fr", "0644444444",
         "3 rue des Lilas", "Bordeaux", "33000", "France"},
        {"Moreau", "Hugo", "hugo.moreau@example.fr", "0655555555",
         "17 boulevard Victor", "Lille", "59000", "France"}
    };
    static const LigneCommande lignesAline[] = {
        {.refProduit = "PRD-0001", .designation = "Clavier mecanique", .prixUnitaireHT = 79.90f, .quantite = 1},
        {.refProduit = "PRD-0005", .designation = "Enceinte Bluetooth", .prixUnitaireHT = 59.90f, .quantite = 1},
        {.refProduit = "PRD-0006", .designation = "Cable HDMI 2 m", .prixUnitaireHT = 10.10f, .quantite = 1}
    };
    static const LigneCommande lignesAnnulee[] = {
        {.refProduit = "PRD-0006", .designation = "Cable HDMI 2 m", .prixUnitaireHT = 10.10f, .quantite = 4}
    };
    static const LigneCommande lignesKarim[] = {
        {.refProduit = "PRD-0004", .designation = "Casque audio", .prixUnitaireHT = 89.50f, .quantite = 1}
    };
    static const LigneCommande lignesSophie[] = {
        {.refProduit = "PRD-0002", .designation = "Souris sans fil", .prixUnitaireHT = 24.50f, .quantite = 1},
        {.refProduit = "PRD-0006", .designation = "Cable HDMI 2 m", .prixUnitaireHT = 10.10f, .quantite = 1}
    };
    static const LigneCommande lignesJulie[] = {
        {.refProduit = "PRD-0002", .designation = "Souris sans fil", .prixUnitaireHT = 24.50f, .quantite = 1},
        {.refProduit = "PRD-0005", .designation = "Enceinte Bluetooth", .prixUnitaireHT = 59.90f, .quantite = 2},
        {.refProduit = "PRD-0007", .designation = "Hub USB-C 7 ports", .prixUnitaireHT = 49.90f, .quantite = 1}
    };
    static const LigneCommande lignesHugo[] = {
        {.refProduit = "PRD-0003", .designation = "Ecran 27 pouces", .prixUnitaireHT = 199.00f, .quantite = 2},
        {.refProduit = "PRD-0008", .designation = "Disque SSD 1 To", .prixUnitaireHT = 109.00f, .quantite = 2},
        {.refProduit = "PRD-0001", .designation = "Clavier mecanique", .prixUnitaireHT = 79.90f, .quantite = 2}
    };
    static const CommandeDemo commandesDemo[NB_COMMANDES_DEMO] = {
        {1, EN_COURS, lignesAline, 3},
        {1, ANNULEE, lignesAnnulee, 1},
        {2, LIVREE, lignesKarim, 1},
        {3, ANNULEE, lignesSophie, 2},
        {4, EXPEDIEE, lignesJulie, 3},
        {5, LIVREE, lignesHugo, 3}
    };
    CodeRetour resultat;
    int i;

    if (boutique == NULL)
        return ERR_INVALIDE;

    strcpy(donnees.nom, "ShopFlow Demo");
    donnees.idClientSuivant = 1;
    donnees.numeroSuivant = 1001;

    for (i = 0; i < NB_PRODUITS_DEMO; i++) {
        resultat = ajouterProduit(&donnees, &produitsDemo[i]);
        if (resultat != OK)
            goto erreur;
    }

    for (i = 0; i < NB_CLIENTS_DEMO; i++) {
        resultat = ajouterClientDemo(&donnees, &clientsDemo[i]);
        if (resultat != OK)
            goto erreur;
    }

    for (i = 0; i < NB_COMMANDES_DEMO; i++) {
        resultat = ajouterCommandeDemo(&donnees, &commandesDemo[i]);
        if (resultat != OK)
            goto erreur;
    }

    *boutique = donnees;
    return OK;

erreur:
    detruireBoutique(&donnees);
    return resultat;
}

void detruireBoutique(Boutique *boutique)
{
    int i;

    if (boutique == NULL)
        return;

    if (boutique->clients != NULL && boutique->nbClients >= 0
        && boutique->nbClients <= boutique->capaciteClients) {
        for (i = 0; i < boutique->nbClients; i++)
            libererClient(&boutique->clients[i]);
    }

    free(boutique->clients);
    free(boutique->produits);
    memset(boutique, 0, sizeof *boutique);
}
