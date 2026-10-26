# Proposition de structure : Catalogue produits et Clients

Organisation proposée pour les modules `Catalogue-produit/` et `Clients/`, tirée du sujet ShopFlow (sections 2, 3, 5 et 6). Rien n'est encore codé : c'est une base à valider en binôme.

---

## 1. Principes communs aux deux modules

### Séparer la logique de la saisie

Chaque fonctionnalité est découpée en deux fonctions :

| Niveau | Rôle | Exemple | Appelée par |
|---|---|---|---|
| **Métier** | Vérifie les règles, modifie la mémoire, renvoie un code. Aucun `printf` ni `scanf`. | `ajouterProduit(b, &p)` | la fonction de saisie, `initialisation.c` |
| **Saisie** | Demande les valeurs, appelle la fonction métier, affiche le résultat. | `menuAjouterProduit(b)` | le `switch` de `main.c` |

Ce découpage a deux avantages :
- `initialisation.c` charge le jeu de démo en appelant directement `ajouterProduit` et `ajouterClient`, sans dupliquer les contrôles.
- Chaque fonction métier se teste seule, comme le demande la section 10 du sujet.

### Codes de retour (dans `types.h`)

```c
typedef enum {
    OK = 0,
    ERR_MEMOIRE,      /* malloc / realloc a renvoye NULL */
    ERR_DOUBLON,      /* reference ou email deja utilise */
    ERR_INTROUVABLE,  /* reference ou identifiant inconnu */
    ERR_INVALIDE,     /* donnee hors contraintes */
    ERR_INTERDIT      /* regle metier non respectee (suppression refusee...) */
} CodeRetour;
```

### Tableaux dynamiques

Le catalogue et le répertoire clients suivent le même schéma. Quand `nb == capacite`, on double la capacité. On passe par un pointeur temporaire pour ne pas perdre le tableau si `realloc` échoue :

```c
/* interne a produit.c (static, absent du .h) */
static CodeRetour agrandirCatalogue(Boutique *b)
{
    int nouvelleCapacite = (b->capaciteProduits == 0) ? CAPACITE_INITIALE
                                                      : b->capaciteProduits * 2;
    Produit *tmp = realloc(b->produits, nouvelleCapacite * sizeof(Produit));

    if (tmp == NULL)
        return ERR_MEMOIRE;          /* b->produits reste valide */

    b->produits = tmp;
    b->capaciteProduits = nouvelleCapacite;
    return OK;
}
```

> ⚠️ `realloc` peut **déplacer** le tableau. Un `Produit *` ou un `Client *` récupéré avant un ajout peut donc devenir invalide après. Il ne faut jamais garder ces pointeurs d'une opération à l'autre. On garde l'indice, ou on refait la recherche. C'est une question classique du jury.

### Dépendances entre modules

Les `.h` n'incluent que `types.h`, donc il n'y a jamais d'inclusion circulaire. Les `.c` peuvent inclure les `.h` des autres modules :

```
produit.c   -> utils.h
client.c    -> utils.h, commande.h   (historique dans la fiche client)
commande.c  -> utils.h, produit.h, client.h   (stock + points de fidelite)
```

---

## 2. Ce qu'il faut dans `types.h` pour ces deux modules

```c
/* Tailles des chaines (taille du tableau, '\0' compris) */
#define TAILLE_REFERENCE    12
#define TAILLE_DESIGNATION  60
#define TAILLE_CATEGORIE    30
#define TAILLE_NOM          50
#define TAILLE_EMAIL        80
#define TAILLE_TELEPHONE    15
#define TAILLE_RUE          80
#define TAILLE_VILLE        50
#define TAILLE_CODE_POSTAL   6
#define TAILLE_PAYS         50

#define CAPACITE_INITIALE    4   /* premiere allocation d'un tableau dynamique */

typedef struct {
    char rue[TAILLE_RUE];
    char ville[TAILLE_VILLE];
    char codePostal[TAILLE_CODE_POSTAL];
    char pays[TAILLE_PAYS];
} Adresse;

typedef struct {
    char  reference[TAILLE_REFERENCE];
    char  designation[TAILLE_DESIGNATION];
    char  categorie[TAILLE_CATEGORIE];
    float prixUnitaireHT;
    int   quantiteStock;
    int   seuilAlerte;
    int   actif;                 /* 1 = actif, 0 = supprime (logique) */
} Produit;

typedef struct {
    int       idClient;
    char      nom[TAILLE_NOM];
    char      prenom[TAILLE_NOM];
    char      email[TAILLE_EMAIL];
    char      telephone[TAILLE_TELEPHONE];
    Adresse   adresse;
    int       pointsFidelite;
    int       nbCommandes;
    int       capaciteCommandes;
    Commande *commandes;         /* NULL tant qu'aucune commande */
    int       actif;
} Client;

typedef struct {
    char     nom[TAILLE_NOM];
    Produit *produits;
    int      nbProduits;
    int      capaciteProduits;
    Client  *clients;
    int      nbClients;
    int      capaciteClients;
    int      numeroSuivant;      /* prochain numero de commande */
    int      idClientSuivant;    /* prochain identifiant client */
} Boutique;
```

Quelques remarques sur ce code :
- **Ordre de déclaration dans `types.h`** : `Date`, `Adresse`, `Produit`, `LigneCommande`, `Commande`, `Client`, `Boutique`. `Client` contient un `Commande *`, donc `Commande` doit être déclarée avant. C'est précisément pour ça que le sujet impose un `types.h` commun.
- **Ordre des champs de `Produit`** : il doit rester celui du sujet (référence, désignation, catégorie, prix, stock, seuil, actif). Sinon, l'initialisation `{"PRD-0001", "Clavier mecanique", ...}` de la page 7 ne correspond plus.

---

## 3. Module Catalogue produits (`produit.h` / `produit.c`)

### Fonctions métier

| Fonction | Rôle | Règle du sujet |
|---|---|---|
| `int trouverProduit(const Boutique *b, const char *ref)` | Renvoie l'indice du produit, ou -1. Cherche **aussi parmi les désactivés**. | Unicité « y compris désactivés » (§5.8) |
| `CodeRetour ajouterProduit(Boutique *b, const Produit *p)` | Valide les champs, agrandit si besoin, copie le produit avec `actif = 1`. | Référence unique et non vide, prix > 0, stock ≥ 0, seuil ≥ 0 |
| `CodeRetour modifierProduit(Boutique *b, const char *ref, float prix, int stock, int seuil)` | Modifie **uniquement** le prix, le stock et le seuil. | Mêmes contraintes, produit actif |
| `CodeRetour supprimerProduit(Boutique *b, const char *ref)` | Passe `actif` à 0. | Refusée si le produit est dans une commande non ANNULEE |
| `float valeurStock(const Boutique *b)` | Σ prix × stock des produits actifs. | « Valorisation du stock » |
| `CodeRetour retirerStock(Boutique *b, const char *ref, int qte)` | Décrémente le stock. | Appelée par `commande.c` (création, ajout de ligne) |
| `CodeRetour remettreStock(Boutique *b, const char *ref, int qte)` | Réintègre le stock. | Appelée par `commande.c` (annulation, suppression de ligne) |

Il y a aussi deux fonctions internes (`static`, absentes du `.h`) :
- `agrandirCatalogue` (voir §1) ;
- `produitDansCommandeActive`, qui parcourt clients → commandes → lignes pour `supprimerProduit`.

### Fonctions d'affichage et de saisie

| Fonction | Menu |
|---|---|
| `void afficherProduit(const Produit *p)` : une ligne formatée | — |
| `void afficherCatalogue(const Boutique *b)` : produits actifs et valeur totale du stock | 4 |
| `void afficherAlertesStock(const Boutique *b)` : produits actifs avec **stock ≤ seuil** | 19 |
| `void menuAjouterProduit(Boutique *b)` | 1 |
| `void menuModifierProduit(Boutique *b)` | 2 |
| `void menuSupprimerProduit(Boutique *b)` | 3 |
| `void menuRechercherProduit(const Boutique *b)` : sous-choix « référence exacte » ou « mot-clé » | 5 |

### Points d'attention

- **Recherche par mot-clé** : elle doit être insensible à la casse et chercher le mot dans la désignation. `strcasestr` n'est pas du C standard. Il faut donc écrire `contientMotCle` dans `utils.c` (copie en minuscules, puis `strstr`).
- **Produit désactivé** : il reste dans le tableau, à la même place. Il n'apparaît plus dans le catalogue ni dans les alertes et ne peut plus être commandé, mais sa référence reste réservée.
- **Prix figé** : changer le prix d'un produit ne modifie pas les anciennes commandes, parce que `LigneCommande` garde sa propre copie de la désignation et du prix.
- **Alertes de stock** : le sujet les range dans `produit.c` (« alertes de réapprovisionnement »). Le choix 19 du menu appelle donc une fonction de ce module, même s'il apparaît sous « Statistiques ».

---

## 4. Module Clients (`client.h` / `client.c`)

### Fonctions métier

| Fonction | Rôle | Règle du sujet |
|---|---|---|
| `int trouverClient(const Boutique *b, int idClient)` | Renvoie l'indice du client, ou -1 (désactivés inclus). | — |
| `int emailExiste(const Boutique *b, const char *email, int idIgnore)` | Renvoie 1 si un **autre** client que `idIgnore` utilise déjà cet email. Mettre 0 pour ne rien ignorer. Désactivés inclus. | Unicité de l'email (§5.8) |
| `CodeRetour ajouterClient(Boutique *b, const Client *c, int *idAttribue)` | Valide, puis attribue `idClientSuivant++`. Met `pointsFidelite`, `nbCommandes` et `capaciteCommandes` à 0, `commandes` à NULL et `actif` à 1. | Identifiant jamais saisi (§5.9) |
| `CodeRetour modifierClient(Boutique *b, int idClient, const Client *infos)` | Copie le nom, le prénom, l'email, le téléphone et l'adresse. Ne touche **pas** à l'id, aux points ni aux commandes. | Email unique (hors lui-même) |
| `CodeRetour supprimerClient(Boutique *b, int idClient)` | Passe `actif` à 0. | Refusée si une commande n'est pas ANNULEE (**LIVREE compte**) |
| `float chiffreAffairesClient(const Client *c)` | Σ des montants des commandes non annulées. | Réutilisée par le palmarès clients |
| `void ajouterPointsFidelite(Client *c, float montantTTC)` | `+= montantTTC / 10` (partie entière). | Appelée par `commande.c` à la création |
| `void retirerPointsFidelite(Client *c, float montantTTC)` | Même calcul en négatif, sans jamais descendre sous 0. | Appelée à l'annulation |
| `void libererClient(Client *c)` | Libère les `lignes` de chaque commande, puis `commandes`. | Appelée par `detruireBoutique` |

### Fonctions d'affichage et de saisie

| Fonction | Menu |
|---|---|
| `void saisirAdresse(Adresse *a)` : réutilisée pour l'ajout et la modification | — |
| `void afficherClient(const Client *c)` : id, nom, email, nombre de commandes, CA | — |
| `void afficherListeClients(const Boutique *b)` | 9 |
| `void menuAjouterClient(Boutique *b)` | 6 |
| `void menuModifierClient(Boutique *b)` | 7 |
| `void menuSupprimerClient(Boutique *b)` | 8 |
| `void menuAfficherClient(const Boutique *b)` : coordonnées, adresse, points, puis historique (une ligne par commande, via une fonction de `commande.c`) | 10 |

### Points d'attention

- **Copie superficielle** : `Client` contient un pointeur. Dans `ajouterClient`, il faut forcer `commandes = NULL` après la copie. Sinon, deux clients pourraient partager le même tableau, ce qui mène à un double `free`.
- **Points de fidélité** : si on ajoute ou supprime une ligne d'une commande EN_COURS, le TTC change. `commande.c` doit alors retirer les anciens points puis ajouter les nouveaux.
- **Arrondi** : il faut arrondir le TTC au centime avant de diviser par 10. Sinon, un `float` qui vaut 29.9999 au lieu de 30.00 donne 2 points au lieu de 3.
- **Validation** : l'email doit contenir `@`, le téléphone exactement 10 chiffres, le code postal exactement 5 chiffres. La rue, la ville, le pays, le nom et le prénom ne doivent pas être vides.
- **`nbCommandes`** compte toutes les commandes, annulées comprises. Le CA, lui, exclut les annulées.

---

## 5. Fonctions dont ces modules ont besoin dans `utils.c`

| Fonction | Rôle |
|---|---|
| `void lireChaine(const char *invite, char *dest, int taille)` | `fgets`, retire le `\n`, vide le buffer si la saisie est trop longue |
| `int lireEntier(const char *invite, int min, int max)` | Redemande tant que la valeur n'est pas un entier dans [min, max] |
| `float lireReel(const char *invite)` | Redemande tant que la saisie n'est pas un nombre |
| `int estVide(const char *s)` | Chaîne vide ou uniquement des espaces |
| `int estNumerique(const char *s, int longueur)` | Exactement `longueur` chiffres (code postal : 5, téléphone : 10) |
| `int estEmailValide(const char *s)` | Contient un `@` |
| `int contientMotCle(const char *texte, const char *motCle)` | Recherche insensible à la casse |

La lecture du choix dans `afficherMenu()` pourra ensuite passer par `lireEntier`.

---

## 6. Branchement dans `main.c`

`main` déclare une `Boutique boutique;`. Il appelle `initialiserDonneesTest(&boutique)` au début et `detruireBoutique(&boutique)` à la fin. Chaque `case` du `switch` appelle ensuite la fonction correspondante :

| Choix | Appel |
|---|---|
| 1 – 3 | `menuAjouterProduit` / `menuModifierProduit` / `menuSupprimerProduit` (`&boutique`) |
| 4 | `afficherCatalogue(&boutique)` |
| 5 | `menuRechercherProduit(&boutique)` |
| 6 – 8 | `menuAjouterClient` / `menuModifierClient` / `menuSupprimerClient` (`&boutique`) |
| 9 | `afficherListeClients(&boutique)` |
| 10 | `menuAfficherClient(&boutique)` |
| 19 | `afficherAlertesStock(&boutique)` |

---

## 7. Lien avec la section 5 du sujet (approche progressive)

Le sujet impose de respecter l'ordre de la section 5 et de tester chaque étape avant de passer à la suivante. Voici ce que chaque point implique pour ces deux modules :

| Point §5 | Ce que ça implique pour Produits / Clients | Fonctions concernées |
|---|---|---|
| 1. Composition | `Client` contient une `Adresse` (un champ, pas un pointeur). Elle est copiée avec le client et il n'y a rien à libérer. | `types.h`, `saisirAdresse` |
| 2. Agrégation | Concerne uniquement `Commande` et `LigneCommande`. | module Commandes |
| 3. Association | `commande.c` retrouve le client propriétaire via son `idClient`, puis range la commande dans son tableau `commandes`. Un client désactivé ne peut plus commander. | `trouverClient` |
| 4. Référence | `commande.c` vérifie que `refProduit` existe et que le produit est actif, puis fige la désignation et le prix dans la ligne. | `trouverProduit` |
| 5. Calcul en cascade | Au niveau Client, on additionne les commandes non annulées. Le niveau Boutique (`statistique.c`) additionne ensuite les clients. | `chiffreAffairesClient` |
| 6 et 7. Cohérence du stock | Depuis une commande, seules ces deux fonctions touchent à `quantiteStock`. `retirerStock` refuse si la quantité dépasse le stock disponible. | `retirerStock`, `remettreStock` |
| 8. Suppression logique | Les contrôles d'unicité parcourent **tous** les enregistrements, sans regarder `actif`. | `trouverProduit`, `emailExiste` |
| 9. Unicité globale | L'id vient de `idClientSuivant`. La saisie ne demande jamais d'id. | `ajouterClient` |
| 10. Unicité des lignes | Côté commande : si le produit est déjà dans la commande, on augmente la ligne existante au lieu d'en créer une deuxième. Côté produit : la première quantité a déjà été retirée du stock, donc `retirerStock` ne vérifie que la quantité ajoutée. Cela revient à contrôler la quantité cumulée. | module Commandes, `retirerStock` |

### Ordre de développement proposé pour ces deux modules

Chaque étape est testée (et passée sous valgrind) avant la suivante :

1. **`types.h`** : toutes les structures, dans l'ordre du §2. Test : le projet compile sans avertissement.
2. **`utils.c`** : `lireChaine`, `lireEntier`, `lireReel` et les fonctions `est...`. Test : saisies invalides (lettres, ligne trop longue, valeur hors bornes).
3. **Produits, ajout et affichage** : `trouverProduit`, `ajouterProduit`, `afficherCatalogue`. Test : ajouter plus de `CAPACITE_INITIALE` produits pour déclencher un `realloc`, puis vérifier qu'il n'y a aucune fuite.
4. **Clients, ajout et affichage** : `trouverClient`, `emailExiste`, `ajouterClient`, `afficherListeClients`. Test : un email en double doit être refusé, y compris celui d'un client désactivé.
5. **Modification, suppression, recherche, alertes** pour les deux modules.
6. **Stock et points de fidélité** : `retirerStock`, `remettreStock` et les fonctions de points. On les teste quand le module Commandes existe (points 6, 7 et 10).

---

## 8. Comment tester le code

On teste à cinq niveaux, du plus simple au plus complet : compilation, mémoire, tests unitaires, scénarios dans le menu, scénarios rejoués automatiquement.

### 8.1 Installer de quoi compiler (une seule fois)

`gcc`, `make` et `valgrind` ne sont pas installés sur le PC. Valgrind ne tourne que sous Linux. Le plus simple est donc **WSL** (Ubuntu dans Windows).

Dans PowerShell, en administrateur, puis redémarrer :
```powershell
wsl --install
```

Ensuite, dans le terminal Ubuntu :
```bash
sudo apt update
sudo apt install build-essential valgrind gdb
cd /mnt/c/Users/quent/projet-algo
code .
```

`code .` rouvre VS Code connecté à WSL, ce qui fait disparaître l'erreur IntelliSense « cannot open source file string.h ».

### 8.2 Compiler : zéro avertissement

```bash
make clean && make
```

Le sujet exige une compilation **sans erreur ni avertissement**. Une seule ligne `warning:` doit être corrigée, pas ignorée. Il faut toujours refaire `make clean && make` avant un dépôt.

### 8.3 Vérifier la mémoire avec valgrind

```bash
make valgrind
```

On navigue dans le menu, puis on quitte **avec 0**. Avec Ctrl+C, `detruireBoutique` n'est jamais appelée et valgrind signale de fausses fuites. Le résultat attendu à la fin est :

```
==1234== All heap blocks were freed -- no leaks are possible
==1234== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

| Message valgrind | Cause probable |
|---|---|
| `definitely lost` | Un `malloc` / `realloc` sans `free` correspondant. Souvent les `lignes` ou les `commandes` d'un client oubliées dans `libererClient`. |
| `Invalid read` / `Invalid write` | Accès hors du tableau : boucle `i <= nb` au lieu de `i < nb`, ou ancien pointeur utilisé après un `realloc`. |
| `Conditional jump ... uninitialised value` | Champ jamais rempli, par exemple `actif` ou `commandes` non initialisé dans `ajouterClient`. |

Grâce à l'option `-g` du Makefile, valgrind indique le fichier et la ligne exacte du problème.

### 8.4 Tests unitaires des fonctions métier

Les fonctions métier ne font aucune saisie (voir §1). On peut donc les appeler depuis un petit programme de test, avec `assert`, sans taper quoi que ce soit au clavier. Il faut créer `tests/test_produit_client.c` dès que les fonctions existent :

```c
/* tests/test_produit_client.c - tests des fonctions metier, sans saisie clavier */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "produit.h"
#include "client.h"

/* Boutique vide, comme au demarrage avant le jeu de demonstration */
static Boutique boutiqueVide(void)
{
    Boutique b = {0};
    strcpy(b.nom, "Test");
    b.numeroSuivant = 1001;
    b.idClientSuivant = 1;
    return b;
}

static void testProduits(void)
{
    Boutique b = boutiqueVide();
    Produit p = {"PRD-0001", "Clavier mecanique", "Informatique", 79.90f, 42, 10, 1};

    assert(ajouterProduit(&b, &p) == OK);
    assert(ajouterProduit(&b, &p) == ERR_DOUBLON);         /* reference deja prise */

    /* Depasser CAPACITE_INITIALE pour forcer au moins un realloc */
    for (int i = 2; i <= 10; i++) {
        snprintf(p.reference, sizeof p.reference, "PRD-%04d", i);
        assert(ajouterProduit(&b, &p) == OK);
    }
    assert(b.nbProduits == 10);
    assert(trouverProduit(&b, "PRD-0007") == 6);
    assert(trouverProduit(&b, "PRD-9999") == -1);

    /* Unicite y compris desactives (§5.8) */
    assert(supprimerProduit(&b, "PRD-0001") == OK);
    assert(b.produits[0].actif == 0);
    strcpy(p.reference, "PRD-0001");
    assert(ajouterProduit(&b, &p) == ERR_DOUBLON);

    /* Contraintes de saisie */
    strcpy(p.reference, "PRD-0099");
    p.prixUnitaireHT = 0.0f;
    assert(ajouterProduit(&b, &p) == ERR_INVALIDE);       /* prix > 0 obligatoire */

    free(b.produits);
    printf("OK  testProduits\n");
}

static void testClients(void)
{
    Boutique b = boutiqueVide();
    Client c = {0};
    int id;

    strcpy(c.nom, "Bernard");
    strcpy(c.prenom, "Julie");
    strcpy(c.email, "j.bernard@mail.fr");
    strcpy(c.telephone, "0612345678");
    strcpy(c.adresse.rue, "12 rue de la Paix");
    strcpy(c.adresse.ville, "Paris");
    strcpy(c.adresse.codePostal, "75002");
    strcpy(c.adresse.pays, "France");

    assert(ajouterClient(&b, &c, &id) == OK);
    assert(id == 1 && b.idClientSuivant == 2);             /* id venant du compteur (§5.9) */
    assert(b.clients[0].commandes == NULL);
    assert(ajouterClient(&b, &c, &id) == ERR_DOUBLON);     /* meme email */

    assert(supprimerClient(&b, 1) == OK);                  /* aucune commande : autorise */
    assert(ajouterClient(&b, &c, &id) == ERR_DOUBLON);     /* email d'un client desactive (§5.8) */

    strcpy(c.email, "pas-un-email");
    assert(ajouterClient(&b, &c, &id) == ERR_INVALIDE);    /* pas de @ */

    strcpy(c.email, "autre@mail.fr");
    strcpy(c.telephone, "06123");
    assert(ajouterClient(&b, &c, &id) == ERR_INVALIDE);    /* pas 10 chiffres */

    for (int i = 0; i < b.nbClients; i++)
        libererClient(&b.clients[i]);
    free(b.clients);
    printf("OK  testClients\n");
}

int main(void)
{
    testProduits();
    testClients();
    printf("Tous les tests sont passes.\n");
    return 0;
}
```

Si un `assert` échoue, le programme s'arrête et affiche le fichier et la ligne du test raté. Ce programme se passe aussi sous valgrind : un test qui réussit mais qui fuit n'est pas réussi.

Cible à ajouter au `Makefile`. Elle compile tous les modules sauf `main.c` avec le fichier de test :

```make
TEST_EXEC = test_shopflow
TEST_OBJ  = tests/test_produit_client.o $(filter-out main.o,$(OBJ))

$(TEST_EXEC): $(TEST_OBJ)
	$(CC) $(TEST_OBJ) -o $@

test: $(TEST_EXEC)
	valgrind --leak-check=full --error-exitcode=1 ./$(TEST_EXEC)
```

Il faut aussi ajouter `test` dans `.PHONY`, et `$(TEST_EXEC) tests/*.o tests/*.d` dans `clean`. Ensuite, la commande `make test` lance tout.

### 8.5 Scénarios à tester dans le menu

Ces scénarios vérifient la partie saisie et les messages affichés. Ce sont aussi ceux à montrer pendant la soutenance.

**Menu**

| Saisie au menu | Résultat attendu |
|---|---|
| `abc`, `-3`, `99`, Entrée vide | « Choix invalide », le menu revient, pas de boucle infinie |
| Ctrl+D (Linux) ou Ctrl+Z puis Entrée (Windows) | Le programme quitte proprement |

**Catalogue produits**

| Choix | Ce qu'on saisit | Résultat attendu |
|---|---|---|
| 1 | Une référence qui existe déjà (`PRD-0001`) | Refusé : doublon |
| 1 | Prix `0`, `-5` ou `abc` | Refusé ou redemandé, sans plantage |
| 1 | Une désignation de 200 caractères | Tronquée ou refusée, et le menu suivant n'est pas perturbé |
| 1 puis 4 | Plusieurs produits au-delà de la capacité | Tous visibles dans le catalogue (`realloc` OK) |
| 2 puis 19 | Mettre un stock sous son seuil | Le produit apparaît dans les alertes |
| 3 | Un produit présent dans une commande EN_COURS ou LIVREE | Refusé |
| 3 | Un produit présent uniquement dans des commandes ANNULEE | Accepté, il disparaît du catalogue |
| 3 puis 1 | Réajouter la référence du produit supprimé | Refusé (§5.8) |
| 5 | Mot-clé `CLAV` | Trouve « Clavier mecanique » (insensible à la casse) |
| 19 | — | Au moins les 2 produits en alerte du jeu de démo |

**Clients**

| Choix | Ce qu'on saisit | Résultat attendu |
|---|---|---|
| 6 | Un email déjà utilisé, même par un client supprimé | Refusé |
| 6 | Email sans `@`, téléphone de 9 chiffres ou avec lettres, code postal `7500` | Refusé |
| 6 | Nom ou rue vide (Entrée directe) | Refusé |
| 6 puis 9 | Deux nouveaux clients | Ids attribués automatiquement et consécutifs |
| 7 | Garder son propre email | Accepté |
| 7 | Prendre l'email d'un autre client | Refusé |
| 8 | Un client avec une commande LIVREE | Refusé |
| 8 | Un client sans commande, ou avec uniquement des commandes ANNULEE | Accepté, il disparaît de la liste |
| 9 | — | Nombre de commandes, et CA **sans** les commandes annulées |
| 10 | Un id qui n'existe pas | Message d'erreur, retour au menu |

### 8.6 Rejouer un scénario automatiquement

On peut rediriger l'entrée du programme pour ne pas retaper les mêmes saisies à chaque test. Si la saisie se termine sans `0`, `afficherMenu()` renvoie 0 en fin d'entrée et le programme quitte quand même proprement.

```bash
# Affichages seuls : catalogue, liste clients, alertes, puis quitter
printf "4\n9\n19\n0\n" | valgrind --leak-check=full ./shopflow

# Scenario avec saisies : une reponse par ligne, dans l'ordre ou le programme les demande
valgrind --leak-check=full ./shopflow < tests/scenario_ajout_produit.txt
```

C'est le moyen le plus rapide de vérifier qu'une modification n'a rien cassé avant un dépôt.

---

## 9. Points à trancher en binôme

1. **CA d'un client : HT ou TTC ?** Proposition : le montant HT après remise (`montantHT`), qui est la convention commerciale, avec le TTC affiché à côté. À justifier pendant la soutenance.
2. **Email insensible à la casse ?** `Julie@mail.fr` et `julie@mail.fr` sont-ils le même client ? Proposition : oui.
3. **Produits désactivés dans le catalogue ?** On peut les masquer, ou les afficher avec la mention « inactif ». Proposition : les masquer, et ajouter une option pour les voir.
4. **Capacité initiale et croissance** : proposition `CAPACITE_INITIALE = 4`, puis doublement à chaque agrandissement.
