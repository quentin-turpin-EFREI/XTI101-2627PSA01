/* Outils d'affichage des statistiques de la boutique. */
#ifndef STATISTIQUE_H
#define STATISTIQUE_H

#include "types.h"

void afficherChiffreAffaires(const Boutique *boutique);
void afficherChiffreAffairesPeriode(const Boutique *boutique,
                                    int annee, int mois);
void afficherPalmaresProduits(const Boutique *boutique, int limite);
void afficherPalmaresClients(const Boutique *boutique);
void afficherStatistiquesParCategorie(const Boutique *boutique);

#endif /* STATISTIQUE_H */
