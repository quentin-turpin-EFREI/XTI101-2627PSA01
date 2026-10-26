/* commande.h - commandes : creation, gestion des lignes, calcul des montants */
#ifndef COMMANDE_H
#define COMMANDE_H

#include "types.h"
/* Création et gestion des lignes d'une commande */
int creerCommande(Boutique *boutique, int idClient, int *numeroCommande);

int ajouterLigneCommande(Boutique *boutique, int idClient,
                         int numeroCommande, const char *refProduit,
                         int quantite);

int supprimerLigneCommande(Boutique *boutique, int idClient,
                           int numeroCommande, const char *refProduit);

/* Statut, annulation et calcul du montant */
int changerStatutCommande(Boutique *boutique, int idClient,
                          int numeroCommande, StatutCommande nouveauStatut);

int annulerCommande(Boutique *boutique, int idClient, int numeroCommande);

void recalculerMontantsCommande(Commande *commande);
#endif /* COMMANDE_H */
