/* initialisation.h - jeu de donnees de demonstration charge en memoire */
#ifndef INITIALISATION_H
#define INITIALISATION_H

#include "types.h"

/* Charge les clients et commandes de demonstration. */
CodeRetour initialiserDonneesTest(Boutique *boutique);

/* Libere les commandes, lignes et tableaux alloues de la boutique. */
void detruireBoutique(Boutique *boutique);

#endif /* INITIALISATION_H */
