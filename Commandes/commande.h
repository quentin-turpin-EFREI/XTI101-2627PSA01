/* commande.h - commandes : creation, gestion des lignes, calcul des montants */
#ifndef COMMANDE_H
#define COMMANDE_H

#include "types.h"
/* Fonctions disponibles du module Commandes. */
CodeRetour creerCommande(Boutique *boutique, int idClient,
                         int *numeroCommande);
CodeRetour ajouterLigneCommande(Boutique *boutique, int idClient,
                                int numeroCommande, const char *refProduit,
                                int quantite);
CodeRetour supprimerLigneCommande(Boutique *boutique, int idClient,
                                  int numeroCommande, const char *refProduit);
CodeRetour changerStatutCommande(Boutique *boutique, int idClient,
                                 int numeroCommande,
                                 StatutCommande nouveauStatut);
CodeRetour annulerCommande(Boutique *boutique, int idClient,
                           int numeroCommande);
void recalculerMontantsCommande(Commande *commande);
void afficherHistoriqueCommandes(const Client *client);
void afficherFacture(const Commande *commande);

#endif /* COMMANDE_H */
