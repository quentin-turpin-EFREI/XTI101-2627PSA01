/* utils.c - saisie securisee, validation des donnees, affichage formate */
#include "utils.h"

int dateValide(Date date)
{
    static const int joursParMois[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    int limite;
    int bissextile;

    if (date.annee < 2000 || date.annee > 2100
        || date.mois < 1 || date.mois > 12)
        return 0;
    bissextile = date.annee % 400 == 0
        || (date.annee % 4 == 0 && date.annee % 100 != 0);
    limite = joursParMois[date.mois - 1];
    if (date.mois == 2 && bissextile)
        limite++;
    return date.jour >= 1 && date.jour <= limite;
}
