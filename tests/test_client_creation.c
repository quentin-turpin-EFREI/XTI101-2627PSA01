#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Clients/client.h"
#include "Commandes/commande.h"
#include "Initialisation/initialisation.h"

static Client clientValide(void)
{
    Client client = {0};

    strcpy(client.nom, "Bernard");
    strcpy(client.prenom, "Julie");
    strcpy(client.email, "julie.bernard@example.fr");
    strcpy(client.telephone, "0612345678");
    strcpy(client.adresse.rue, "12 rue de la Paix");
    strcpy(client.adresse.ville, "Paris");
    strcpy(client.adresse.codePostal, "75002");
    strcpy(client.adresse.pays, "France");
    client.pointsFidelite = 150;
    client.nbCommandes = 3;
    client.capaciteCommandes = 4;
    client.commandes = NULL;
    client.actif = 0;

    return client;
}

static void testAjoutEtValeursInitiales(void)
{
    Boutique boutique = {0};
    Client client = clientValide();
    int id = -1;

    boutique.idClientSuivant = 1;
    client.commandes = malloc(1);
    assert(client.commandes != NULL);
    assert(ajouterClient(&boutique, &client, &id) == OK);
    free(client.commandes); /* l'ajout ne prend pas possession de ce pointeur */
    assert(id == 1);
    assert(boutique.idClientSuivant == 2);
    assert(boutique.nbClients == 1);
    assert(boutique.clients[0].idClient == 1);
    assert(boutique.clients[0].pointsFidelite == 0);
    assert(boutique.clients[0].nbCommandes == 0);
    assert(boutique.clients[0].capaciteCommandes == 0);
    assert(boutique.clients[0].commandes == NULL);
    assert(boutique.clients[0].actif == 1);

    free(boutique.clients);
    puts("OK testAjoutEtValeursInitiales");
}

static void testEmailDupliqueInsensibleALaCasse(void)
{
    Boutique boutique = {0};
    Client client = clientValide();
    int id;

    boutique.idClientSuivant = 10;
    assert(ajouterClient(&boutique, &client, &id) == OK);
    boutique.clients[0].actif = 0;
    strcpy(client.email, "JULIE.BERNARD@EXAMPLE.FR");

    assert(ajouterClient(&boutique, &client, &id) == ERR_DOUBLON);
    assert(boutique.nbClients == 1);
    assert(boutique.idClientSuivant == 11);

    free(boutique.clients);
    puts("OK testEmailDupliqueInsensibleALaCasse");
}

static void testValidationChamps(void)
{
    Boutique boutique = {0};
    Client client = clientValide();
    int id;

    boutique.idClientSuivant = 1;
    strcpy(client.nom, "   ");
    assert(ajouterClient(&boutique, &client, &id) == ERR_INVALIDE);

    client = clientValide();
    client.email[0] = '\0';
    assert(ajouterClient(&boutique, &client, &id) == ERR_INVALIDE);

    client = clientValide();
    strcpy(client.email, "email-invalide");
    assert(ajouterClient(&boutique, &client, &id) == ERR_INVALIDE);

    client = clientValide();
    strcpy(client.email, "julie@@example.fr");
    assert(ajouterClient(&boutique, &client, &id) == ERR_INVALIDE);

    client = clientValide();
    strcpy(client.email, "@example.fr");
    assert(ajouterClient(&boutique, &client, &id) == ERR_INVALIDE);

    client = clientValide();
    strcpy(client.email, "julie@");
    assert(ajouterClient(&boutique, &client, &id) == ERR_INVALIDE);

    client = clientValide();
    strcpy(client.email, "julie bernard@example.fr");
    assert(ajouterClient(&boutique, &client, &id) == ERR_INVALIDE);

    client = clientValide();
    strcpy(client.nom, "   ");
    assert(ajouterClient(&boutique, &client, &id) == ERR_INVALIDE);

    client = clientValide();
    client.prenom[0] = '\0';
    assert(ajouterClient(&boutique, &client, &id) == ERR_INVALIDE);

    client = clientValide();
    strcpy(client.telephone, "06123x5678");
    assert(ajouterClient(&boutique, &client, &id) == ERR_INVALIDE);

    client = clientValide();
    strcpy(client.telephone, "061234567");
    assert(ajouterClient(&boutique, &client, &id) == ERR_INVALIDE);

    client = clientValide();
    strcpy(client.telephone, "06123456789");
    assert(ajouterClient(&boutique, &client, &id) == ERR_INVALIDE);

    client = clientValide();
    client.telephone[0] = '\0';
    assert(ajouterClient(&boutique, &client, &id) == ERR_INVALIDE);

    client = clientValide();
    strcpy(client.adresse.codePostal, "7500");
    assert(ajouterClient(&boutique, &client, &id) == ERR_INVALIDE);

    client = clientValide();
    strcpy(client.adresse.codePostal, "75A02");
    assert(ajouterClient(&boutique, &client, &id) == ERR_INVALIDE);

    client = clientValide();
    client.adresse.codePostal[0] = '\0';
    assert(ajouterClient(&boutique, &client, &id) == ERR_INVALIDE);

    client = clientValide();
    client.adresse.rue[0] = '\0';
    assert(ajouterClient(&boutique, &client, &id) == ERR_INVALIDE);

    assert(boutique.nbClients == 0);
    assert(boutique.clients == NULL);
    free(boutique.clients);
    puts("OK testValidationChamps");
}

static void testAgrandissementDuRepertoire(void)
{
    Boutique boutique = {0};
    Client client = clientValide();
    int id;
    int i;

    boutique.idClientSuivant = 1;
    for (i = 0; i < CAPACITE_INITIALE + 1; i++) {
        snprintf(client.email, sizeof client.email,
                 "client%d@example.fr", i);
        assert(ajouterClient(&boutique, &client, &id) == OK);
        assert(id == i + 1);
    }

    assert(boutique.nbClients == CAPACITE_INITIALE + 1);
    assert(boutique.capaciteClients >= boutique.nbClients);
    assert(strcmp(boutique.clients[CAPACITE_INITIALE].email,
                  "client4@example.fr") == 0);

    free(boutique.clients);
    puts("OK testAgrandissementDuRepertoire");
}

static void testModifierClient(void)
{
    Boutique boutique = {0};
    Client client = clientValide();
    Client infos = clientValide();
    Client second = clientValide();
    int id;

    boutique.idClientSuivant = 1;
    assert(ajouterClient(&boutique, &client, &id) == OK);
    boutique.clients[0].pointsFidelite = 42;

    strcpy(infos.nom, "Martin");
    strcpy(infos.prenom, "Julie");
    strcpy(infos.telephone, "0711223344");
    strcpy(infos.adresse.rue, "5 avenue Victor Hugo");
    strcpy(infos.adresse.ville, "Lyon");
    strcpy(infos.adresse.codePostal, "69002");
    strcpy(infos.adresse.pays, "France");
    assert(modifierClient(&boutique, id, &infos) == OK);
    assert(strcmp(boutique.clients[0].nom, "Martin") == 0);
    assert(strcmp(boutique.clients[0].telephone, "0711223344") == 0);
    assert(strcmp(boutique.clients[0].adresse.ville, "Lyon") == 0);
    assert(boutique.clients[0].idClient == id);
    assert(boutique.clients[0].pointsFidelite == 42);
    assert(boutique.clients[0].nbCommandes == 0);
    assert(boutique.clients[0].commandes == NULL);

    strcpy(second.email, "autre.client@example.fr");
    assert(ajouterClient(&boutique, &second, &id) == OK);
    strcpy(infos.email, second.email);
    assert(modifierClient(&boutique, 1, &infos) == ERR_DOUBLON);
    assert(strcmp(boutique.clients[0].email, "julie.bernard@example.fr") == 0);
    assert(modifierClient(&boutique, 999, &infos) == ERR_INTROUVABLE);

    free(boutique.clients);
    puts("OK testModifierClient");
}

static void testSupprimerClientSelonCommandes(void)
{
    Boutique boutique = {0};
    Client client = clientValide();
    Commande commandes[2] = {{0}};
    int id;

    boutique.idClientSuivant = 1;
    assert(ajouterClient(&boutique, &client, &id) == OK);
    boutique.clients[0].commandes = commandes;
    boutique.clients[0].nbCommandes = 1;
    boutique.clients[0].capaciteCommandes = 2;

    commandes[0].statut = EN_COURS;
    assert(supprimerClient(&boutique, id) == ERR_INTERDIT);
    assert(boutique.clients[0].actif == 1);

    commandes[0].statut = LIVREE;
    assert(supprimerClient(&boutique, id) == ERR_INTERDIT);

    commandes[0].statut = ANNULEE;
    assert(supprimerClient(&boutique, id) == OK);
    assert(boutique.clients[0].actif == 0);
    assert(trouverClient(&boutique, id) == 0);
    assert(emailExiste(&boutique, client.email, 0) == 1);
    assert(supprimerClient(&boutique, id) == ERR_INTROUVABLE);

    boutique.clients[0].commandes = NULL;
    free(boutique.clients);
    puts("OK testSupprimerClientSelonCommandes");
}

static void testChiffreAffairesEtListe(void)
{
    Boutique boutique = {0};
    Client client = clientValide();
    Commande commandes[3] = {{0}};
    int id;

    boutique.idClientSuivant = 1;
    assert(ajouterClient(&boutique, &client, &id) == OK);
    commandes[0].statut = EN_COURS;
    commandes[0].montantHT = 50.0f;
    commandes[1].statut = LIVREE;
    commandes[1].montantHT = 25.5f;
    commandes[2].statut = ANNULEE;
    commandes[2].montantHT = 1000.0f;
    boutique.clients[0].commandes = commandes;
    boutique.clients[0].nbCommandes = 3;
    boutique.clients[0].capaciteCommandes = 3;

    assert(chiffreAffairesClient(&boutique.clients[0]) == 75.5f);
    afficherListeClients(&boutique);

    boutique.clients[0].commandes = NULL;
    free(boutique.clients);
    puts("OK testChiffreAffairesEtListe");
}

static void testInitialisationDonneesDemo(void)
{
    Boutique boutique = {0};
    int nombreCommandes = 0;
    int commandesAvecTroisLignes = 0;
    int commandesAuDessusDe600 = 0;
    int statutsVus[4] = {0};
    int alertesStock = 0;
    int categoriesDifferentes = 0;
    int i;

    assert(initialiserDonneesTest(&boutique) == OK);
    assert(boutique.nbProduits == NB_PRODUITS_DEMO);
    assert(boutique.nbClients == NB_CLIENTS_DEMO);
    assert(boutique.idClientSuivant == NB_CLIENTS_DEMO + 1);
    assert(boutique.numeroSuivant == 1001 + NB_COMMANDES_DEMO);

    for (i = 0; i < boutique.nbProduits; i++) {
        int j;
        int nouvelleCategorie = 1;

        if (boutique.produits[i].quantiteStock
            <= boutique.produits[i].seuilAlerte)
            alertesStock++;
        for (j = 0; j < i; j++) {
            if (strcmp(boutique.produits[i].categorie,
                       boutique.produits[j].categorie) == 0) {
                nouvelleCategorie = 0;
                break;
            }
        }
        categoriesDifferentes += nouvelleCategorie;
    }
    assert(alertesStock >= 2);
    assert(categoriesDifferentes >= 3);

    for (i = 0; i < boutique.nbClients; i++) {
        int j;

        assert(boutique.clients[i].idClient == i + 1);
        assert(boutique.clients[i].adresse.rue[0] != '\0');
        assert(boutique.clients[i].adresse.ville[0] != '\0');
        assert(boutique.clients[i].adresse.codePostal[0] != '\0');
        assert(boutique.clients[i].adresse.pays[0] != '\0');
        nombreCommandes += boutique.clients[i].nbCommandes;
        for (j = 0; j < boutique.clients[i].nbCommandes; j++) {
            const Commande *commande = &boutique.clients[i].commandes[j];

            assert(commande->statut >= EN_COURS
                   && commande->statut <= ANNULEE);
            statutsVus[commande->statut] = 1;
            commandesAvecTroisLignes += commande->nbLignes >= 3;
            commandesAuDessusDe600 += commande->montantHT > 600.0f;
        }
    }
    assert(nombreCommandes == NB_COMMANDES_DEMO);
    assert(commandesAvecTroisLignes >= 2);
    assert(commandesAuDessusDe600 >= 1);
    assert(statutsVus[EN_COURS]);
    assert(statutsVus[LIVREE]);
    assert(statutsVus[ANNULEE]);
    assert(statutsVus[EXPEDIEE]);

    assert(boutique.clients[0].nbCommandes == 2);
    assert(chiffreAffairesClient(&boutique.clients[0]) > 149.89f);
    assert(chiffreAffairesClient(&boutique.clients[0]) < 149.91f);
    assert(boutique.clients[1].nbCommandes == 1);
    assert(chiffreAffairesClient(&boutique.clients[1]) == 89.50f);
    assert(boutique.clients[2].nbCommandes == 1);
    assert(chiffreAffairesClient(&boutique.clients[2]) == 0.0f);
    assert(supprimerClient(&boutique, 2) == ERR_INTERDIT);
    assert(supprimerClient(&boutique, 3) == OK);

    detruireBoutique(&boutique);
    assert(boutique.clients == NULL);
    assert(boutique.nbClients == 0);
    puts("OK testInitialisationDonneesDemo");
}

static void testCreerCommande(void)
{
    Boutique boutique = {0};
    LigneCommande lignes[2] = {0};
    int numero;
    Commande *commande;

    assert(initialiserDonneesTest(&boutique) == OK);
    assert(trouverClient(&boutique, 1) == 0);
    assert(creerCommande(&boutique, 1, &numero) == OK);
    assert(numero == 1007);
    assert(boutique.numeroSuivant == 1008);
    assert(boutique.clients[0].nbCommandes == 3);

    commande = &boutique.clients[0].commandes[2];
    assert(commande->statut == EN_COURS);
    assert(commande->montantHT == 0.0f);
    assert(commande->lignes == NULL);

    lignes[0].prixUnitaireHT = 19.95f;
    lignes[0].quantite = 2;
    lignes[1].prixUnitaireHT = 4.05f;
    lignes[1].quantite = 3;
    commande->lignes = lignes;
    commande->nbLignes = 2;
    commande->capaciteLignes = 2;
    recalculerMontantsCommande(commande);
    assert(commande->montantHT > 52.04f && commande->montantHT < 52.06f);
    commande->lignes = NULL; /* lignes est un tableau local au test */

    assert(supprimerClient(&boutique, 3) == OK);
    assert(creerCommande(&boutique, 3, &numero) == ERR_INTROUVABLE);
    assert(creerCommande(&boutique, 999, &numero) == ERR_INTROUVABLE);

    detruireBoutique(&boutique);
    puts("OK testCreerCommande");
}

int main(void)
{
    testAjoutEtValeursInitiales();
    testEmailDupliqueInsensibleALaCasse();
    testValidationChamps();
    testAgrandissementDuRepertoire();
    testModifierClient();
    testSupprimerClientSelonCommandes();
    testChiffreAffairesEtListe();
    testInitialisationDonneesDemo();
    testCreerCommande();
    puts("Tous les tests clients et commandes sont passes.");
    return 0;
}
