/* initialisation.h - jeu de donnees de demonstration charge en memoire */
#ifndef INITIALISATION_H
#define INITIALISATION_H

#include "types.h"

#define NB_PRODUITS_DEMO 8
#define NB_CLIENTS_DEMO 5
#define NB_COMMANDES_DEMO 6

/* Charge les donnees de demonstration dans une Boutique initialisee a zero.
 * En cas d'erreur, renvoie un code ERR_* sans modifier la boutique. */
CodeRetour initialiserDonneesTest(Boutique *boutique);

/* Libere toutes les commandes, lignes et tableaux alloues de la boutique. */
void detruireBoutique(Boutique *boutique);

#endif /* INITIALISATION_H */
