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
    CodeRetour resultat;

    if (lireEntier("Identifiant du client : ", &idClient) != 1)
        return;
    resultat = creerCommande(boutique, idClient, &numero);
    if (resultat == OK)
        printf("Commande %d creee (sans ligne pour le moment).\n", numero);
    else if (resultat == ERR_INTROUVABLE)
        puts("Client introuvable.");
    else
        puts("Impossible de creer la commande.");
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

static void afficherChiffreAffaires(const Boutique *boutique)
{
    float total = 0.0f;
    int i;

    for (i = 0; i < boutique->nbClients; i++)
        if (boutique->clients[i].actif)
            total += chiffreAffairesClient(&boutique->clients[i]);
    printf("Chiffre d'affaires global : %.2f EUR HT\n", total);
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

static void afficherMenu(void)
{
    puts("\n========================================");
    puts("           SHOPFLOW - MENU");
    puts("========================================");
    puts("\n--- Catalogue produits ---");
    puts("  1. Ajouter un produit");
    puts("  2. Mettre a jour un produit");
    puts("  3. Supprimer un produit");
    puts("  4. Afficher le catalogue");
    puts("  5. Rechercher un produit");
    puts("\n--- Clients ---");
    puts("  6. Ajouter un client");
    puts("  7. Mettre a jour un client");
    puts("  8. Supprimer un client");
    puts("  9. Afficher la liste des clients");
    puts(" 10. Afficher la fiche d'un client");
    puts("\n--- Commandes ---");
    puts(" 11. Creer une commande");
    puts(" 12. Ajouter / supprimer une ligne (non disponible)");
    puts(" 13. Changer le statut d'une commande (non disponible)");
    puts(" 14. Annuler une commande (non disponible)");
    puts(" 15. Afficher l'historique d'un client");
    puts("\n--- Statistiques ---");
    puts(" 16. Chiffre d'affaires global");
    puts(" 17. Palmares produits (non disponible)");
    puts(" 18. Palmares clients (non disponible)");
    puts(" 19. Alertes de stock");
    puts("\n  0. Quitter");
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
            case 1: ajouterProduitInteractif(&boutique); break;
            case 2: modifierProduitInteractif(&boutique); break;
            case 3: supprimerProduitInteractif(&boutique); break;
            case 4: afficherCatalogue(&boutique); attendreEntree(); break;
            case 5: rechercherProduit(&boutique); break;
            case 6: menuAjouterClient(&boutique); break;
            case 7: menuModifierClient(&boutique); break;
            case 8: menuSupprimerClient(&boutique); break;
            case 9: afficherListeClients(&boutique); attendreEntree(); break;
            case 10: afficherFicheClient(&boutique); break;
            case 11: creerCommandeInteractive(&boutique); break;
            case 12: case 13: case 14:
                puts("Cette fonctionnalite n'est pas encore implementee.");
                break;
            case 15: afficherHistoriqueClient(&boutique); break;
            case 16: afficherChiffreAffaires(&boutique); break;
            case 17: case 18:
                puts("Cette fonctionnalite n'est pas encore implementee.");
                break;
            case 19: afficherAlertesStock(&boutique); break;
            case 0: puts("Au revoir !"); break;
            default: puts("Choix invalide, recommencez."); break;
        }
    } while (choix != 0);

    detruireBoutique(&boutique);
    return EXIT_SUCCESS;
}
