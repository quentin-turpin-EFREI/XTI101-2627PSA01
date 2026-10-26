#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Clients/client.h"

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
    strcpy(client.email, "email-invalide");
    assert(ajouterClient(&boutique, &client, &id) == ERR_INVALIDE);

    client = clientValide();
    strcpy(client.telephone, "06123x5678");
    assert(ajouterClient(&boutique, &client, &id) == ERR_INVALIDE);

    client = clientValide();
    strcpy(client.adresse.codePostal, "7500");
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

int main(void)
{
    testAjoutEtValeursInitiales();
    testEmailDupliqueInsensibleALaCasse();
    testValidationChamps();
    testAgrandissementDuRepertoire();
    puts("Tous les tests de creation client sont passes.");
    return 0;
}
