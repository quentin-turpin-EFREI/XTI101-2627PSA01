CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -g
CPPFLAGS += -I . -I Catalogue-produit -I Clients -I Commandes \
            -I Statistiques -I Saisie-validation -I Initialisation

BUILD_DIR ?= build
APP := $(BUILD_DIR)/shopflow

APP_SOURCES := \
	main.c \
	Catalogue-produit/produit.c \
	Clients/client.c \
	Commandes/commande.c \
	Statistiques/statistique.c \
	Saisie-validation/utils.c \
	Initialisation/initialisation.c

APP_HEADERS := \
	types.h \
	Catalogue-produit/produit.h \
	Clients/client.h \
	Commandes/commande.h \
	Statistiques/statistique.h \
	Saisie-validation/utils.h \
	Initialisation/initialisation.h

.PHONY: all run test clean

all: $(APP)

$(APP): $(APP_SOURCES) $(APP_HEADERS)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(APP_SOURCES) -o $@

$(BUILD_DIR)/test_client_creation: tests/test_client_creation.c \
		Catalogue-produit/produit.c Clients/client.c Commandes/commande.c \
		Saisie-validation/utils.c Initialisation/initialisation.c $(APP_HEADERS)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_client_creation.c \
		Catalogue-produit/produit.c Clients/client.c Commandes/commande.c \
		Saisie-validation/utils.c Initialisation/initialisation.c -o $@

$(BUILD_DIR)/test_produit: tests/test_produit.c \
		Catalogue-produit/produit.c Catalogue-produit/produit.h types.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_produit.c \
		Catalogue-produit/produit.c -o $@

$(BUILD_DIR)/test_commande_facturation: tests/test_commande_facturation.c \
		Commandes/commande.c Clients/client.c Saisie-validation/utils.c \
		$(APP_HEADERS)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_commande_facturation.c \
		Commandes/commande.c Clients/client.c Saisie-validation/utils.c -o $@

run: $(APP)
	$(APP)

test: $(APP) $(BUILD_DIR)/test_client_creation \
		$(BUILD_DIR)/test_produit $(BUILD_DIR)/test_commande_facturation
	$(BUILD_DIR)/test_client_creation
	$(BUILD_DIR)/test_produit
	$(BUILD_DIR)/test_commande_facturation
	bash tests/test_input_validation.sh $(APP)

clean:
	rm -f $(APP) $(BUILD_DIR)/test_client_creation \
		$(BUILD_DIR)/test_produit $(BUILD_DIR)/test_commande_facturation
