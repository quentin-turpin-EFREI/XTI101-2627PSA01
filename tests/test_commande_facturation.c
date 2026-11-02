#include <assert.h>
#include <stdio.h>

#include "types.h"
#include "Commandes/commande.h"

static void verifierProche(float obtenu, float attendu)
{
    float ecart = obtenu - attendu;

    if (ecart < 0.0f)
        ecart = -ecart;
    assert(ecart < 0.001f);
}

static void verifierFacture(float brut, float tauxRemise,
                            float remise, float totalHT,
                            float tva, float port, float ttc)
{
    LigneCommande ligne = {0};
    Commande commande = {0};

    if (brut > 0.0f) {
        ligne.prixUnitaireHT = brut;
        ligne.quantite = 1;
        commande.lignes = &ligne;
        commande.nbLignes = 1;
    }

    recalculerMontantsCommande(&commande);

    verifierProche(commande.montantBrutHT, brut);
    verifierProche(commande.tauxRemise, tauxRemise);
    verifierProche(commande.montantRemise, remise);
    verifierProche(commande.montantHT, totalHT);
    verifierProche(commande.montantTVA, tva);
    verifierProche(commande.fraisPort, port);
    verifierProche(commande.montantTTC, ttc);
    if (brut > 0.0f)
        verifierProche(ligne.sousTotalHT, brut);
}

static void testCommandeVideSansFraisDePort(void)
{
    verifierFacture(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    puts("OK commande vide : aucun frais de port");
}

static void testFraisDePortEtSeuilGratuit(void)
{
    verifierFacture(59.99f, 0.0f, 0.0f, 59.99f,
                    11.998f, 5.90f, 77.888f);
    verifierFacture(60.0f, 0.0f, 0.0f, 60.0f,
                    12.0f, 0.0f, 72.0f);
    puts("OK frais de port : payants sous 60 EUR, gratuits a partir de 60 EUR");
}

static void testRemisesProgressivesEtTVA(void)
{
    verifierFacture(99.99f, 0.0f, 0.0f, 99.99f,
                    19.998f, 0.0f, 119.988f);
    verifierFacture(100.0f, 5.0f, 5.0f, 95.0f,
                    19.0f, 0.0f, 114.0f);
    verifierFacture(299.99f, 5.0f, 14.9995f, 284.9905f,
                    56.9981f, 0.0f, 341.9886f);
    verifierFacture(300.0f, 10.0f, 30.0f, 270.0f,
                    54.0f, 0.0f, 324.0f);
    verifierFacture(599.99f, 10.0f, 59.999f, 539.991f,
                    107.9982f, 0.0f, 647.9892f);
    verifierFacture(600.0f, 15.0f, 90.0f, 510.0f,
                    102.0f, 0.0f, 612.0f);
    puts("OK remises progressives et TVA a 20 % apres remise");
}

int main(void)
{
    testCommandeVideSansFraisDePort();
    testFraisDePortEtSeuilGratuit();
    testRemisesProgressivesEtTVA();
    puts("Tous les tests de facturation sont passes.");
    return 0;
}
