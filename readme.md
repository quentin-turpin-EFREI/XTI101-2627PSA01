# ShopFlow — gestion de boutique en ligne

[![CI ShopFlow](https://github.com/quentin-turpin-EFREI/XTI101-2627PSA01/actions/workflows/ci.yml/badge.svg?branch=MAIN)](https://github.com/quentin-turpin-EFREI/XTI101-2627PSA01/actions/workflows/ci.yml)

ShopFlow est une application en C qui gère le catalogue d'une boutique, ses clients et leurs commandes. Le projet met en pratique la programmation modulaire et l'allocation dynamique. Les données sont conservées en mémoire pendant l'exécution et chargées depuis le jeu de démonstration au démarrage.

## Fonctionnalités

- Gestion des produits : ajout, modification, suppression logique, recherche par référence ou désignation, alertes de stock.
- Gestion des clients : création, modification, suppression logique, adresses complètes, historique et points fidélité.
- Gestion des commandes : lignes, stock, factures, statuts `EN_COURS`, `EXPEDIEE`, `LIVREE` et `ANNULEE`.
- Règles de facturation : remises progressives de 0 %, 5 %, 10 % et 15 % selon le montant brut HT ; TVA de 20 % après remise ; port de 5,90 € offert dès 60 € HT brut.
- Fidélité : un point par tranche entière de 10 € TTC, port compris. Les points sont recalculés lors de l'ajout ou du retrait d'une ligne et retirés à l'annulation.
- Statistiques : chiffre d'affaires HT/TTC global et par période, palmarès des produits et clients, ventes par catégorie.
- Recherche avancée des produits par catégorie et fourchette de prix.

Les commandes annulées sont exclues du chiffre d'affaires et des quantités vendues. L'annulation restitue le stock. La progression des statuts suit `EN_COURS → EXPEDIEE → LIVREE` ; seule une commande `EN_COURS` peut être annulée.

## Organisation du code

- `Catalogue-produit/` : catalogue, stock et recherche.
- `Clients/` : clients, adresses, fidélité et historique.
- `Commandes/` : lignes de commande, statuts et facturation.
- `Statistiques/` : chiffre d'affaires et classements.
- `Saisie-validation/` : validations communes, dont les dates.
- `Initialisation/` : données de démonstration et libération de la boutique.
- `types.h` : structures partagées.
- `main.c` : menus et point d'entrée.

Le jeu initial comprend 8 produits, 5 clients et 6 commandes couvrant les quatre statuts autorisés.

## Compilation

Avec GCC ou Clang, depuis la racine du dépôt :

```sh
mkdir -p build
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror \
  -I . -I Catalogue-produit -I Clients -I Commandes \
  -I Statistiques -I Saisie-validation -I Initialisation \
  main.c Catalogue-produit/produit.c Clients/client.c \
  Commandes/commande.c Statistiques/statistique.c \
  Saisie-validation/utils.c Initialisation/initialisation.c \
  -o build/shopflow
./build/shopflow
```

La CI compile également le projet avec GCC et Clang, lance les tests unitaires et vérifie les fuites mémoire avec Valgrind.

## Contributeurs

Auteurs recensés dans l'historique Git du dépôt :

- George-Adrien
- Quentinthierry12
- theop
- Usopp
