/* Point d'entree de ShopFlow : menu principal et appels aux modules. */
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "produit.h"
#include "client.h"
#include "commande.h"
#include "initialisation.h"
#include "statistique.h"

/* Lit une ligne complete et vide le reste si elle depasse le tampon. */
static int lireTexte(const char *invite, char *texte, size_t capacite)
{
    size_t longueur;
    int caractere;

    fputs(invite, stdout);
    if (fgets(texte, (int)capacite, stdin) == NULL)
        return 0;

    longueur = strlen(texte);
    if (longueur > 0 && texte[longueur - 1] == '\n') {
        texte[--longueur] = '\0';
    } else {
        caractere = getchar();
        if (caractere != '\n' && caractere != EOF) {
            do {
                caractere = getchar();
            } while (caractere != '\n' && caractere != EOF);
            puts("Saisie trop longue.");
            return -1;
        }
    }

    if (longueur > 0 && texte[longueur - 1] == '\r')
        texte[--longueur] = '\0';
    return 1;
}

static int lireEntier(const char *invite, int *valeur)
{
    char texte[64];
    char *fin;
    long resultat;
    int statut = lireTexte(invite, texte, sizeof texte);

    if (statut <= 0)
        return statut;
    errno = 0;
    resultat = strtol(texte, &fin, 10);
    while (isspace((unsigned char)*fin))
        fin++;
    if (texte[0] == '\0' || strlen(texte) > 15 || *fin != '\0' || errno == ERANGE
        || resultat < -2147483647L - 1L || resultat > 2147483647L) {
        puts("Choix invalide, recommencez.");
        return -1;
    }
    *valeur = (int)resultat;
    return 1;
}

static int lireReel(const char *invite, float *valeur)
{
    char texte[64];
    char *fin;
    float resultat;
    int statut = lireTexte(invite, texte, sizeof texte);

    if (statut <= 0)
        return statut;
    errno = 0;
    resultat = strtof(texte, &fin);
    while (isspace((unsigned char)*fin))
        fin++;
    if (texte[0] == '\0' || *fin != '\0' || errno == ERANGE
        || !isfinite(resultat)) {
        puts("Valeur numerique invalide.");
        return -1;
    }
    *valeur = resultat;
    return 1;
}

static int lireChoix(void)
{
    int choix;
    int statut = lireEntier("Votre choix : ", &choix);

    return statut == 1 ? choix : (statut == 0 ? 0 : -1);
}

static void attendreEntree(void)
{
    char ligne[8];
    (void)lireTexte("Appuyez sur Entree pour continuer...", ligne, sizeof ligne);
}

static void ajouterProduitInteractif(Boutique *boutique)
{
    Produit produit = {0};
    CodeRetour resultat;

    puts("\n--- Ajout d'un produit ---");
    if (lireTexte("Reference : ", produit.reference, sizeof produit.reference) != 1
        || lireTexte("Designation : ", produit.designation,
                     sizeof produit.designation) != 1
        || lireTexte("Categorie : ", produit.categorie,
                     sizeof produit.categorie) != 1
        || lireReel("Prix unitaire HT : ", &produit.prixUnitaireHT) != 1
        || lireEntier("Quantite en stock : ", &produit.quantiteStock) != 1
        || lireEntier("Seuil d'alerte : ", &produit.seuilAlerte) != 1)
        return;

    resultat = ajouterProduit(boutique, &produit);
    switch (resultat) {
        case OK: puts("Produit ajoute."); break;
        case ERR_DOUBLON: puts("Cette reference existe deja."); break;
        case ERR_MEMOIRE: puts("Memoire insuffisante pour ajouter le produit."); break;
        default: puts("Produit invalide; il n'a pas ete ajoute."); break;
    }
}

static void modifierProduitInteractif(Boutique *boutique)
{
    char reference[TAILLE_REFERENCE];
    float prix;
    int stock, seuil;
    int indice;
    CodeRetour resultat;

    if (lireTexte("Reference du produit : ", reference, sizeof reference) != 1)
        return;
    indice = trouverProduit(boutique, reference);
    if (indice == -1 || boutique->produits[indice].actif == 0) {
        puts("Produit introuvable.");
        return;
    }
    if (lireReel("Nouveau prix HT : ", &prix) != 1
        || lireEntier("Nouveau stock : ", &stock) != 1
        || lireEntier("Nouveau seuil d'alerte : ", &seuil) != 1)
        return;
    resultat = modifierProduit(boutique, reference, prix, stock, seuil);
    if (resultat == OK)
        puts("Produit mis a jour.");
    else
        puts("Valeurs invalides; produit non modifie.");
}

static void supprimerProduitInteractif(Boutique *boutique)
{
    char reference[TAILLE_REFERENCE];
    CodeRetour resultat;

    if (lireTexte("Reference du produit : ", reference, sizeof reference) != 1)
        return;
    resultat = supprimerProduit(boutique, reference);
    if (resultat == OK)
        puts("Produit supprime (suppression logique).");
    else if (resultat == ERR_INTERDIT)
        puts("Suppression refusee : une commande non annulee utilise ce produit.");
    else
        puts("Produit introuvable.");
}

static void rechercherProduit(Boutique *boutique)
{
    char recherche[TAILLE_DESIGNATION];

    if (lireTexte("Reference exacte ou mot dans la designation (respecter la casse) : ",
                  recherche, sizeof recherche) != 1)
        return;
    rechercherProduits(boutique, recherche);
}

static void creerCommandeInteractive(Boutique *boutique)
{
    int idClient;
    int numero;
    int nombreLignes;
    int i;
    CodeRetour resultat;

    if (lireEntier("Identifiant du client : ", &idClient) != 1)
        return;
    resultat = creerCommande(boutique, idClient, &numero);
    if (resultat != OK) {
        puts(resultat == ERR_INTROUVABLE
                 ? "Client introuvable."
                 : "Impossible de creer la commande.");
        return;
    }

    printf("Commande %d creee.\n", numero);
    if (lireEntier("Nombre de lignes a ajouter : ", &nombreLignes) != 1
        || nombreLignes <= 0) {
        (void)annulerCommande(boutique, idClient, numero);
        puts("Commande annulee : une commande doit contenir au moins une ligne.");
        return;
    }
    for (i = 0; i < nombreLignes; i++) {
        char reference[TAILLE_REFERENCE];
        int quantite;

        printf("Ligne %d/%d\n", i + 1, nombreLignes);
        if (lireTexte("Reference produit : ", reference,
                      sizeof reference) != 1
            || lireEntier("Quantite : ", &quantite) != 1) {
            (void)annulerCommande(boutique, idClient, numero);
            puts("Commande annulee apres une saisie interrompue.");
            return;
        }
        resultat = ajouterLigneCommande(boutique, idClient, numero,
                                        reference, quantite);
        if (resultat != OK) {
            (void)annulerCommande(boutique, idClient, numero);
            if (resultat == ERR_INTERDIT)
                puts("Stock insuffisant ou quantite invalide; commande annulee.");
            else if (resultat == ERR_INTROUVABLE)
                puts("Client, commande ou produit introuvable; commande annulee.");
            else
                puts("Ajout impossible; commande annulee.");
            return;
        }
    }
    puts("Commande validee et stock mis a jour.");
}

static Commande *lireCommandeCible(Boutique *boutique, int *idClient)
{
    int numero;
    int indiceClient;
    int i;

    if (lireEntier("Identifiant du client : ", idClient) != 1
        || lireEntier("Numero de commande : ", &numero) != 1)
        return NULL;
    indiceClient = trouverClient(boutique, *idClient);
    if (indiceClient < 0 || !boutique->clients[indiceClient].actif) {
        puts("Client introuvable.");
        return NULL;
    }
    for (i = 0; i < boutique->clients[indiceClient].nbCommandes; i++)
        if (boutique->clients[indiceClient].commandes[i].numero == numero)
            return &boutique->clients[indiceClient].commandes[i];
    puts("Commande introuvable.");
    return NULL;
}

static void ajouterLigneInteractive(Boutique *boutique)
{
    int idClient;
    int quantite;
    Commande *commande = lireCommandeCible(boutique, &idClient);
    char reference[TAILLE_REFERENCE];
    CodeRetour resultat;

    if (commande == NULL)
        return;
    if (lireTexte("Reference produit : ", reference, sizeof reference) != 1
        || lireEntier("Quantite : ", &quantite) != 1)
        return;
    resultat = ajouterLigneCommande(boutique, idClient, commande->numero,
                                    reference, quantite);
    if (resultat == OK)
        puts("Ligne ajoutee; stock, facture et fidelite mis a jour.");
    else if (resultat == ERR_INTERDIT)
        puts("Operation interdite : statut, quantite ou stock invalide.");
    else
        puts("Commande ou produit introuvable.");
}

static void supprimerLigneInteractive(Boutique *boutique)
{
    int idClient;
    Commande *commande = lireCommandeCible(boutique, &idClient);
    char reference[TAILLE_REFERENCE];
    CodeRetour resultat;

    if (commande == NULL
        || lireTexte("Reference produit a retirer : ", reference,
                     sizeof reference) != 1)
        return;
    resultat = supprimerLigneCommande(boutique, idClient, commande->numero,
                                      reference);
    if (resultat == OK)
        puts("Ligne retiree; stock, facture et fidelite mis a jour.");
    else if (resultat == ERR_INTERDIT)
        puts("Les lignes ne peuvent etre modifiees que sur une commande EN_COURS.");
    else
        puts("Produit absent de cette commande.");
}

static void changerStatutInteractive(Boutique *boutique)
{
    int idClient;
    int choix;
    Commande *commande = lireCommandeCible(boutique, &idClient);
    StatutCommande nouveauStatut;
    CodeRetour resultat;

    if (commande == NULL)
        return;
    puts("1. Marquer EXPEDIEE\n2. Marquer LIVREE");
    if (lireEntier("Nouveau statut : ", &choix) != 1)
        return;
    if (choix == 1)
        nouveauStatut = EXPEDIEE;
    else if (choix == 2)
        nouveauStatut = LIVREE;
    else {
        puts("Choix de statut invalide.");
        return;
    }
    resultat = changerStatutCommande(boutique, idClient, commande->numero,
                                     nouveauStatut);
    puts(resultat == OK ? "Statut mis a jour."
                        : "Transition refusee : respectez EN_COURS -> EXPEDIEE -> LIVREE.");
}

static void annulerCommandeInteractive(Boutique *boutique)
{
    int idClient;
    Commande *commande = lireCommandeCible(boutique, &idClient);
    CodeRetour resultat;

    if (commande == NULL)
        return;
    resultat = annulerCommande(boutique, idClient, commande->numero);
    if (resultat == OK)
        puts("Commande annulee; stock restitue et points retires.");
    else
        puts("Annulation refusee : seule une commande EN_COURS peut etre annulee.");
}

static void modifierCommandeInteractive(Boutique *boutique)
{
    int choix;

    puts("\n1. Ajouter une ligne\n2. Supprimer une ligne\n3. Changer le statut\n4. Annuler");
    if (lireEntier("Action : ", &choix) != 1)
        return;
    switch (choix) {
        case 1: ajouterLigneInteractive(boutique); break;
        case 2: supprimerLigneInteractive(boutique); break;
        case 3: changerStatutInteractive(boutique); break;
        case 4: annulerCommandeInteractive(boutique); break;
        default: puts("Choix invalide."); break;
    }
}

static void afficherHistoriqueClient(Boutique *boutique)
{
    int idClient;
    int indice;

    if (lireEntier("Identifiant du client : ", &idClient) != 1)
        return;
    indice = trouverClient(boutique, idClient);
    if (indice < 0 || !boutique->clients[indice].actif) {
        puts("Client introuvable.");
        return;
    }
    afficherHistoriqueCommandes(&boutique->clients[indice]);
}

static void afficherAlertesStock(const Boutique *boutique)
{
    int i;
    int nombre = 0;

    puts("\n--- Alertes de stock ---");
    for (i = 0; i < boutique->nbProduits; i++) {
        const Produit *produit = &boutique->produits[i];
        if (produit->actif && produit->quantiteStock <= produit->seuilAlerte) {
            printf("%s - %s : stock %d (seuil %d)\n", produit->reference,
                   produit->designation, produit->quantiteStock,
                   produit->seuilAlerte);
            nombre++;
        }
    }
    if (nombre == 0)
        puts("Aucune alerte de stock.");
}

static void rechercherProduitsAvancee(const Boutique *boutique)
{
    char categorie[TAILLE_CATEGORIE];
    float minimum, maximum;
    int i, nombre = 0;

    if (lireTexte("Categorie (vide pour toutes) : ", categorie,
                  sizeof categorie) != 1
        || lireReel("Prix minimum HT : ", &minimum) != 1
        || lireReel("Prix maximum HT : ", &maximum) != 1)
        return;
    if (minimum < 0.0f || maximum < minimum) {
        puts("Intervalle de prix invalide.");
        return;
    }
    for (i = 0; i < boutique->nbProduits; i++) {
        const Produit *produit = &boutique->produits[i];
        if (!produit->actif || produit->prixUnitaireHT < minimum
            || produit->prixUnitaireHT > maximum
            || (categorie[0] != '\0'
                && strcmp(categorie, produit->categorie) != 0))
            continue;
        printf("%s - %s (%s), %.2f EUR HT, stock %d\n",
               produit->reference, produit->designation, produit->categorie,
               produit->prixUnitaireHT, produit->quantiteStock);
        nombre++;
    }
    if (nombre == 0)
        puts("Aucun produit ne correspond aux criteres.");
}

static void afficherUneCommande(Boutique *boutique)
{
    int idClient;
    int numeroCommande;
    int indiceClient;
    int i;
    const Client *client;

    if (lireEntier("Identifiant du client : ", &idClient) != 1
        || lireEntier("Numero de commande : ", &numeroCommande) != 1)
        return;

    indiceClient = trouverClient(boutique, idClient);
    if (indiceClient < 0 || !boutique->clients[indiceClient].actif) {
        puts("Client introuvable.");
        return;
    }

    client = &boutique->clients[indiceClient];
    for (i = 0; i < client->nbCommandes; i++) {
        if (client->commandes[i].numero == numeroCommande) {
            afficherFacture(&client->commandes[i]);
            return;
        }
    }
    puts("Commande introuvable.");
}

static void menuProduits(Boutique *boutique)
{
    int choix;

    do {
        puts("\n===== GESTION DES PRODUITS =====");
        puts("  1. Ajouter un produit");
        puts("  2. Modifier un produit");
        puts("  3. Supprimer un produit");
        puts("  4. Afficher le catalogue");
        puts("  5. Rechercher un produit");
        puts("  0. Retour au menu principal");
        choix = lireChoix();

        switch (choix) {
            case 1: ajouterProduitInteractif(boutique); break;
            case 2: modifierProduitInteractif(boutique); break;
            case 3: supprimerProduitInteractif(boutique); break;
            case 4: afficherCatalogue(boutique); attendreEntree(); break;
            case 5: rechercherProduit(boutique); break;
            case 0: break;
            default: puts("Choix invalide."); break;
        }
    } while (choix != 0);
}

static void menuClients(Boutique *boutique)
{
    int choix;

    do {
        puts("\n===== GESTION DES CLIENTS =====");
        puts("  1. Ajouter un client");
        puts("  2. Modifier un client");
        puts("  3. Supprimer un client");
        puts("  4. Afficher la liste des clients");
        puts("  5. Afficher la fiche d'un client");
        puts("  6. Afficher les points de fidelite d'un client");
        puts("  0. Retour au menu principal");
        choix = lireChoix();

        switch (choix) {
            case 1: menuAjouterClient(boutique); break;
            case 2: menuModifierClient(boutique); break;
            case 3: menuSupprimerClient(boutique); break;
            case 4: afficherListeClients(boutique); attendreEntree(); break;
            case 5: afficherFicheClient(boutique); break;
            case 6: afficherPointsFideliteClient(boutique); break;
            case 0: break;
            default: puts("Choix invalide."); break;
        }
    } while (choix != 0);
}

static void menuCommandes(Boutique *boutique)
{
    int choix;

    do {
        puts("\n===== GESTION DES COMMANDES =====");
        puts("  1. Creer une commande");
        puts("  2. Modifier une commande (lignes/statut/annulation)");
        puts("  3. Afficher l'historique d'un client");
        puts("  4. Afficher une facture");
        puts("  0. Retour au menu principal");
        choix = lireChoix();

        switch (choix) {
            case 1: creerCommandeInteractive(boutique); break;
            case 2: modifierCommandeInteractive(boutique); break;
            case 3: afficherHistoriqueClient(boutique); break;
            case 4: afficherUneCommande(boutique); break;
            case 0: break;
            default: puts("Choix invalide."); break;
        }
    } while (choix != 0);
}

static void menuStatistiques(Boutique *boutique)
{
    int choix;

    do {
        puts("\n===== STATISTIQUES =====");
        puts("  1. Chiffre d'affaires global (HT et TTC)");
        puts("  2. Chiffre d'affaires par mois ou annee");
        puts("  3. Palmares des 5 produits les plus vendus");
        puts("  4. Palmares des clients");
        puts("  5. Alertes de stock");
        puts("  6. Statistiques de ventes par categorie");
        puts("  7. Recherche produit par categorie et fourchette de prix");
        puts("  0. Retour au menu principal");
        choix = lireChoix();

        switch (choix) {
            case 1: afficherChiffreAffaires(boutique); break;
            case 2: {
                int annee, mois;
                if (lireEntier("Annee (2000-2100) : ", &annee) == 1
                    && lireEntier("Mois (1-12, ou 0 pour l'annee) : ",
                                  &mois) == 1)
                    afficherChiffreAffairesPeriode(boutique, annee, mois);
                break;
            }
            case 3: afficherPalmaresProduits(boutique, 5); break;
            case 4: afficherPalmaresClients(boutique); break;
            case 5: afficherAlertesStock(boutique); break;
            case 6: afficherStatistiquesParCategorie(boutique); break;
            case 7: rechercherProduitsAvancee(boutique); break;
            case 0: break;
            default: puts("Choix invalide."); break;
        }
    } while (choix != 0);
}

static void afficherMenu(void)
{
    puts("\n========================================");
    puts("  SHOPFLOW - Boutique en ligne");
    puts("========================================");
    puts("  1. Gestion des produits");
    puts("  2. Gestion des clients");
    puts("  3. Gestion des commandes");
    puts("  4. Statistiques");
    puts("  0. Quitter");
}

int main(void)
{
    Boutique boutique = {0};
    int choix;

    if (initialiserDonneesTest(&boutique) != OK) {
        fputs("Impossible de charger les donnees de demonstration.\n", stderr);
        return EXIT_FAILURE;
    }

    do {
        afficherMenu();
        choix = lireChoix();
        switch (choix) {
            case 1: menuProduits(&boutique); break;
            case 2: menuClients(&boutique); break;
            case 3: menuCommandes(&boutique); break;
            case 4: menuStatistiques(&boutique); break;
            case 0: puts("Au revoir !"); break;
            default: puts("Choix invalide, recommencez."); break;
        }
    } while (choix != 0);

    detruireBoutique(&boutique);
    return EXIT_SUCCESS;
}
