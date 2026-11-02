#!/usr/bin/env bash
set -euo pipefail

app="${1:-./build/shopflow}"
tmp_dir="$(mktemp -d)"
trap 'rm -rf "$tmp_dir"' EXIT

run_client_case() {
    local case_name="$1"
    shift

    printf '%s\n' 6 "$@" '' 0 | "$app" > "$tmp_dir/$case_name.log"
    grep -Fq 'Erreur : informations invalides.' "$tmp_dir/$case_name.log"
    if grep -Fq "Client cree avec l'identifiant" "$tmp_dir/$case_name.log"; then
        printf 'Le cas client invalide a ete accepte : %s\n' "$case_name" >&2
        exit 1
    fi
}

run_client_case email_sans_arobase \
    Bernard Julie julie.example.fr 0612345678 \
    '12 rue de la Paix' Paris 75002 France
run_client_case email_deux_arobases \
    Bernard Julie julie@@example.fr 0612345678 \
    '12 rue de la Paix' Paris 75002 France
run_client_case email_arobase_initial \
    Bernard Julie @example.fr 0612345678 \
    '12 rue de la Paix' Paris 75002 France
run_client_case email_arobase_final \
    Bernard Julie julie@ 0612345678 \
    '12 rue de la Paix' Paris 75002 France
run_client_case email_avec_espace \
    Bernard Julie 'julie bernard@example.fr' 0612345678 \
    '12 rue de la Paix' Paris 75002 France
run_client_case nom_manquant \
    '' Julie julie@example.fr 0612345678 \
    '12 rue de la Paix' Paris 75002 France
run_client_case prenom_manquant \
    Bernard '' julie@example.fr 0612345678 \
    '12 rue de la Paix' Paris 75002 France
run_client_case prenom_espace \
    Bernard '   ' julie@example.fr 0612345678 \
    '12 rue de la Paix' Paris 75002 France
run_client_case email_manquant \
    Bernard Julie '' 0612345678 \
    '12 rue de la Paix' Paris 75002 France
run_client_case telephone_trop_court \
    Bernard Julie julie@example.fr 061234567 \
    '12 rue de la Paix' Paris 75002 France
run_client_case telephone_avec_lettre \
    Bernard Julie julie@example.fr 06123456x8 \
    '12 rue de la Paix' Paris 75002 France
run_client_case telephone_manquant \
    Bernard Julie julie@example.fr '' \
    '12 rue de la Paix' Paris 75002 France
run_client_case code_postal_trop_court \
    Bernard Julie julie@example.fr 0612345678 \
    '12 rue de la Paix' Paris 7500 France
run_client_case code_postal_avec_lettre \
    Bernard Julie julie@example.fr 0612345678 \
    '12 rue de la Paix' Paris 75A02 France
run_client_case code_postal_manquant \
    Bernard Julie julie@example.fr 0612345678 \
    '12 rue de la Paix' Paris '' France
run_client_case rue_manquante \
    Bernard Julie julie@example.fr 0612345678 '' Paris 75002 France
run_client_case ville_manquante \
    Bernard Julie julie@example.fr 0612345678 \
    '12 rue de la Paix' '' 75002 France
run_client_case pays_manquant \
    Bernard Julie julie@example.fr 0612345678 \
    '12 rue de la Paix' Paris 75002 ''

printf '%s\n' 6 'Client' 'Doublon' 'ALINE.MARTIN@EXAMPLE.FR' '0612345678' \
    '12 rue de la Paix' Paris 75002 France '' 0 \
    | "$app" > "$tmp_dir/email_doublon.log"
grep -Fq 'Erreur : un client utilise deja cette adresse email.' \
    "$tmp_dir/email_doublon.log"

long_name="$(printf '%060d' 1)"
printf '%s\n' 6 "$long_name" '' 0 | "$app" > "$tmp_dir/nom_trop_long.log"
grep -Fq 'Saisie interrompue ou trop longue; client non cree.' \
    "$tmp_dir/nom_trop_long.log"

for invalid_choice in '' abc 1abc 99 -3 999999999999999999999; do
    printf '%s\n' "$invalid_choice" 0 | "$app" > "$tmp_dir/menu.log"
    grep -Fq 'Choix invalide, recommencez.' "$tmp_dir/menu.log"
    grep -Fq 'Au revoir !' "$tmp_dir/menu.log"
done

long_choice="$(printf '%030d' 1)"
printf '%s\n' "$long_choice" 0 | "$app" > "$tmp_dir/choix_trop_long.log"
grep -Fq 'Choix invalide, recommencez.' "$tmp_dir/choix_trop_long.log"

"$app" < /dev/null > "$tmp_dir/fin_entree.log"
grep -Fq 'Au revoir !' "$tmp_dir/fin_entree.log"

printf '%s\n' 7 abc '' 0 | "$app" > "$tmp_dir/identifiant_invalide.log"
grep -Fq 'Identifiant invalide; modification annulee.' \
    "$tmp_dir/identifiant_invalide.log"

echo 'Toutes les validations de saisie client et menu passent.'
