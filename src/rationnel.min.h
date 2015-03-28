#ifndef __RATIONNEL_H__
#define __RATIONNEL_H__
#include <stdbool.h>
#include <stdio.h>
#include "automate.h"
#include "ensemble.h"

typedef enum Noeud {
    EPSILON,
    LETTRE,
    STAR,
    UNION,
    CONCAT
} Noeud;

typedef struct Rationnel {
  Noeud etiquette;
  char lettre;
  struct Rationnel *gauche;
  struct Rationnel *droit;
  struct Rationnel *pere;
  int position_min;
  int position_max;
  void * data;
} Rationnel;

typedef Rationnel *** Systeme;

Rationnel *rationnel(Noeud etiquette, char lettre, int position_min, int position_max, void *data, Rationnel *gauche, Rationnel *droit, Rationnel *pere);
Rationnel *Epsilon();
Rationnel *Lettre(char lettre);
Rationnel *Union(Rationnel* rat1, Rationnel* rat2);
Rationnel *Concat(Rationnel* rat1, Rationnel* rat2);
Rationnel *Star(Rationnel* rat);
 
bool est_racine(Rationnel* rat);

Noeud get_etiquette(Rationnel* rat);
char get_lettre(Rationnel* rat);
int get_position_min(Rationnel* rat);
int get_position_max(Rationnel* rat);

void set_position_min(Rationnel* rat, int valeur);
void set_position_max(Rationnel* rat, int valeur);

Rationnel *fils_gauche(Rationnel* rat);
Rationnel *fils_droit(Rationnel* rat);   
Rationnel *fils(Rationnel* rat);
Rationnel *pere(Rationnel* rat);

void print_rationnel(Rationnel* rat);

Rationnel *expression_to_rationnel(const char *expr);
void rationnel_to_dot(Rationnel *rat, char* nom_fichier);
int rationnel_to_dot_aux(Rationnel *rat, FILE *output, int pere, int noeud_courant);

void numeroter_rationnel(Rationnel *rat);
bool contient_mot_vide(Rationnel *rat);
Ensemble *premier(Rationnel *rat);
Ensemble *dernier(Rationnel *);
Ensemble *suivant(Rationnel *, int);
Automate *Glushkov(Rationnel *rat);
bool meme_langage (const char *expr1, const char* expr2);

Systeme systeme(Automate *automate);
void print_ligne(Rationnel **ligne, int nb_vars);
void print_systeme(Systeme systeme, int taille);
Rationnel **resoudre_variable_arden(Rationnel **ligne, int numero_variable, int nb_vars);
Rationnel **substituer_variable(Rationnel **ligne, int numero_variable, Rationnel **valeur_variable, int n);
Systeme resoudre_systeme(Systeme sys, int nb_vars);
Rationnel *Arden(Automate *automate);

#endif
