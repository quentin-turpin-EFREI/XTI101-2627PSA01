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

static void attendreEntree(void)
{
    int caractere;

    printf("\nAppuyez sur Entree pour revenir au menu principal...");
    fflush(stdout);
    do {
        caractere = getchar();
    } while (caractere != '\n' && caractere != EOF);
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
            case 4:  afficherCatalogue(&boutique); attendreEntree(); break;
            case 5:  printf("[A faire] Rechercher un produit\n"); break;

            /* Clients -> logique metier, saisie et affichage dedies. */
            case 6:
                menuAjouterClient(&boutique);
                attendreEntree();
                break;
            case 7:
                menuModifierClient(&boutique);
                attendreEntree();
                break;
            case 8:
                menuSupprimerClient(&boutique);
                attendreEntree();
                break;
            case 9:
                afficherListeClients(&boutique);
                attendreEntree();
                break;
            case 10:
                afficherFicheClient(&boutique);
                attendreEntree();
                break;

            /* Commandes -> commande.c */
            case 11: printf("[A faire] Creer une commande\n"); break;
            case 12: printf("[A faire] Ajouter / supprimer une ligne\n"); break;
            case 13: printf("[A faire] Changer le statut d'une commande\n"); break;
            case 14: printf("[A faire] Annuler une commande\n"); break;
            case 15: printf("[A faire] Afficher une commande\n"); break;

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
