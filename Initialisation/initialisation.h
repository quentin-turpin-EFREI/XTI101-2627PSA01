/* initialisation.h - jeu de donnees de demonstration charge en memoire */
#ifndef INITIALISATION_H
#define INITIALISATION_H

#include "types.h"

/* Charge un petit jeu de donnees en memoire pour les essais du programme. */
CodeRetour initialiserDonneesTest(Boutique *boutique);

/* Libere les tableaux alloues par l'initialisation et remet la boutique a zero. */
void detruireBoutique(Boutique *boutique);

#endif /* INITIALISATION_H */
