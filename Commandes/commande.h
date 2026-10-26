/* commande.h - commandes : creation, gestion des lignes, calcul des montants */
#ifndef COMMANDE_H
#define COMMANDE_H

#include "types.h"
/* Fonctions disponibles du module Commandes. */
CodeRetour creerCommande(Boutique *boutique, int idClient,
                         int *numeroCommande);
void recalculerMontantsCommande(Commande *commande);

#endif /* COMMANDE_H */
