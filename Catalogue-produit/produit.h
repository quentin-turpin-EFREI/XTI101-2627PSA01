/* produit.h - catalogue produits : CRUD, gestion du stock, alertes de reapprovisionnement */
#ifndef PRODUIT_H
#define PRODUIT_H

#include "../types.h"

/* CodeRetour est declare dans types.h : OK signifie reussite ;
 * les valeurs ERR_... indiquent la raison d'un echec. */

/* Recherche : renvoie un indice, ou -1 si la reference est absente. */
int trouverProduit(const Boutique *boutique, const char *reference);
/* Ajout, modification, suppression logique et gestion du stock. */
CodeRetour ajouterProduit(Boutique *boutique, const Produit *produit);
CodeRetour modifierProduit(Boutique *boutique, const char *reference,
                           float prix, int stock, int seuil);
CodeRetour supprimerProduit(Boutique *boutique, const char *reference);
float valeurStock(const Boutique *boutique);
CodeRetour retirerStock(Boutique *boutique, const char *reference, int quantite);
CodeRetour remettreStock(Boutique *boutique, const char *reference, int quantite);

/* Affichages et saisies au clavier pour le menu principal. */
void afficherProduit(const Produit *produit);
void afficherCatalogue(const Boutique *boutique);
void afficherAlertesStock(const Boutique *boutique);
void menuAjouterProduit(Boutique *boutique);
void menuModifierProduit(Boutique *boutique);
void menuSupprimerProduit(Boutique *boutique);
void menuRechercherProduit(const Boutique *boutique);

#endif /* PRODUIT_H */
