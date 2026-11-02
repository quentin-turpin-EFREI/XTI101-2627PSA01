/* main.c - point d'entree : initialisation de la Boutique,
 * boucle du menu principal et liberation finale de la memoire */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "produit.h"
#include "client.h"
#include "commande.h"
#include "statistique.h"
#include "utils.h"
#include "initialisation.h"


/* Affiche le menu principal et renvoie le choix saisi (-1 si saisie invalide) */
static int afficherMenu(void)
{
    char saisie[16];
    int choix;

    printf("\n========================================\n");
    printf("           SHOPFLOW - MENU\n");
    printf("========================================\n");

    printf("\n--- Catalogue produits ---\n");
    printf("  1. Ajouter un produit\n");
    printf("  2. Mettre a jour un produit\n");
    printf("  3. Supprimer un produit\n");
    printf("  4. Afficher le catalogue\n");
    printf("  5. Rechercher un produit\n");

    printf("\n--- Clients ---\n");
    printf("  6. Ajouter un client\n");
    printf("  7. Mettre a jour un client\n");
    printf("  8. Supprimer un client\n");
    printf("  9. Afficher la liste des clients\n");
    printf(" 10. Afficher la fiche d'un client\n");

    printf("\n--- Commandes ---\n");
    printf(" 11. Creer une commande\n");
    printf(" 12. Ajouter / supprimer une ligne\n");
    printf(" 13. Changer le statut d'une commande\n");
    printf(" 14. Annuler une commande\n");
    printf(" 15. Afficher une commande\n");

    printf("\n--- Statistiques ---\n");
    printf(" 16. Chiffre d'affaires (total / par periode)\n");
    printf(" 17. Palmares produits (top 5)\n");
    printf(" 18. Palmares clients\n");
    printf(" 19. Alertes de stock\n");

    printf("\n  0. Quitter\n");
    printf("========================================\n");
    printf("Votre choix : ");

    /* Lecture de la ligne entiere : une saisie non numerique ne bloque pas le programme */
    if (fgets(saisie, sizeof saisie, stdin) == NULL)
        return 0;   /* fin d'entree (EOF) : on quitte */

    /* Ligne trop longue : on vide le reste du buffer */
    if (strchr(saisie, '\n') == NULL) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }

    if (sscanf(saisie, "%d", &choix) != 1)
        return -1;

    return choix;
}

static int lireEntierCommande(const char *invite, int *valeur)
{
    char saisie[64];
    int c;
    printf("%s", invite);
    if (fgets(saisie, sizeof saisie, stdin) == NULL) return 0;
    if (strchr(saisie, '\n') == NULL)
        while ((c = getchar()) != '\n' && c != EOF) { }
    return sscanf(saisie, "%d", valeur) == 1;
}

static int lireReferenceCommande(char *ref, size_t taille)
{
    size_t longueur;
    int c;
    printf("Reference produit : ");
    if (fgets(ref, (int)taille, stdin) == NULL) return 0;
    longueur = strlen(ref);
    if (longueur > 0 && ref[longueur - 1] == '\n') {
        ref[--longueur] = '\0';
        if (longueur > 0 && ref[longueur - 1] == '\r') ref[--longueur] = '\0';
    } else {
        c = getchar();
        if (c != '\n' && c != EOF) {
            while ((c = getchar()) != '\n' && c != EOF) { }
            return 0;
        }
    }
    return ref[0] != '\0';
}

void menuCreerCommande(Boutique *b)
{
    int id, numero, resultat;
    if (!lireEntierCommande("Identifiant client : ", &id)) { puts("Saisie invalide."); return; }
    resultat = creerCommande(b, id, &numero);
    if (resultat == OK) printf("Commande %d creee.\n", numero);
    else printf("Creation impossible (code %d).\n", resultat);
}

void menuGererLigneCommande(Boutique *b)
{
    int id, numero, action, quantite, resultat;
    char ref[TAILLE_REFERENCE];
    if (!lireEntierCommande("Identifiant client : ", &id) ||
        !lireEntierCommande("Numero de commande : ", &numero) ||
        !lireEntierCommande("1 = ajouter, 2 = supprimer : ", &action) ||
        (action != 1 && action != 2) || !lireReferenceCommande(ref, sizeof ref)) {
        puts("Saisie invalide."); return;
    }
    if (action == 1) {
        if (!lireEntierCommande("Quantite : ", &quantite)) { puts("Saisie invalide."); return; }
        resultat = ajouterLigneCommande(b, id, numero, ref, quantite);
    } else resultat = supprimerLigneCommande(b, id, numero, ref);
    if (resultat == OK) puts("Ligne de commande mise a jour.");
    else printf("Operation impossible (code %d).\n", resultat);
}

void menuChangerStatutCommande(Boutique *b)
{
    int id, numero, choix, resultat;
    StatutCommande statut;
    if (!lireEntierCommande("Identifiant client : ", &id) ||
        !lireEntierCommande("Numero de commande : ", &numero) ||
        !lireEntierCommande("1 = expedier, 2 = livrer, 3 = annuler : ", &choix)) {
        puts("Saisie invalide."); return;
    }
    if (choix == 1) statut = EXPEDIEE;
    else if (choix == 2) statut = LIVREE;
    else if (choix == 3) statut = ANNULEE;
    else { puts("Statut invalide."); return; }
    resultat = changerStatutCommande(b, id, numero, statut);
    if (resultat == OK) puts("Statut mis a jour.");
    else printf("Changement impossible (code %d).\n", resultat);
}

void menuAnnulerCommande(Boutique *b)
{
    int id, numero, resultat;
    if (!lireEntierCommande("Identifiant client : ", &id) ||
        !lireEntierCommande("Numero de commande : ", &numero)) { puts("Saisie invalide."); return; }
    resultat = annulerCommande(b, id, numero);
    if (resultat == OK) puts("Commande annulee; stock et points ajustes.");
    else printf("Annulation impossible (code %d).\n", resultat);
}

void menuAfficherFacture(Boutique *b)
{
    int id, numero, ic, i;
    Client *client;
    if (!lireEntierCommande("Identifiant client : ", &id) ||
        !lireEntierCommande("Numero de commande : ", &numero)) { puts("Saisie invalide."); return; }
    ic = trouverClient(b, id);
    if (ic < 0) { puts("Client introuvable."); return; }
    client = &b->clients[ic];
    for (i = 0; i < client->nbCommandes; i++)
        if (client->commandes[i].numero == numero) {
            afficherFacture(&client->commandes[i]); return;
        }
    puts("Commande introuvable pour ce client.");
}

int main(void)
{
    int choix;
    Boutique boutique = {0};

    if (initialiserDonneesTest(&boutique) != OK) {
        fprintf(stderr, "Impossible de charger les donnees de demonstration.\n");
        return EXIT_FAILURE;
    }

    do {
        choix = afficherMenu();

        switch (choix) {
            /* Catalogue produits -> produit.c */
            case 1:  printf("[A faire] Ajouter un produit\n"); break;
            case 2:  printf("[A faire] Mettre a jour un produit\n"); break;
            case 3:  printf("[A faire] Supprimer un produit\n"); break;
            case 4:  printf("[A faire] Afficher le catalogue\n"); break;
            case 5:  printf("[A faire] Rechercher un produit\n"); break;

            /* Clients -> client.c */
            case 6:  menuAjouterClient(&boutique); break;
            case 7:  menuModifierClient(&boutique); break;
            case 8:  menuSupprimerClient(&boutique); break;
            case 9:  afficherListeClients(&boutique); break;
            case 10: printf("[A faire] Afficher la fiche d'un client\n"); break;

            /* Commandes -> commande.c */
            case 11: menuCreerCommande(&boutique); break;
            case 12: menuGererLigneCommande(&boutique); break;
            case 13: menuChangerStatutCommande(&boutique); break;
            case 14: menuAnnulerCommande(&boutique); break;
            case 15: menuAfficherFacture(&boutique); break;

            /* Statistiques -> statistique.c */
            case 16: printf("[A faire] Chiffre d'affaires\n"); break;
            case 17: printf("[A faire] Palmares produits\n"); break;
            case 18: printf("[A faire] Palmares clients\n"); break;
            case 19: printf("[A faire] Alertes de stock\n"); break;

            case 0:  printf("Au revoir !\n"); break;
            default: printf("Choix invalide, recommencez.\n"); break;
        }
    } while (choix != 0);

    detruireBoutique(&boutique);
    return 0;
}
