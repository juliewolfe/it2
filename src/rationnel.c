/*
 *   Ce fichier fait partie d'un projet de programmation donné en Licence 3 
 *   à l'Université de Bordeaux.
 *
 *   Copyright (C) 2015 Giuliana Bianchi, Adrien Boussicault, Thomas Place, Marc Zeitoun
 *
 *    This Library is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This Library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this Library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "rationnel.h"
#include "ensemble.h"
#include "automate.h"
#include "parse.h"
#include "scan.h"
#include "outils.h"

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

typedef struct data_meme_langage 
{ 
   bool estDansSecondAutomate; 
   Automate* a; 
} Data;

typedef struct data_systeme
{
   int taille;
   Systeme sys;
}Datasys;

int yyparse(Rationnel **rationnel, yyscan_t scanner);


Rationnel *rationnel(Noeud etiquette, char lettre, int position_min, int position_max, void *data, Rationnel *gauche, Rationnel *droit, Rationnel *pere)
{
   Rationnel *rat;
   rat = (Rationnel *) malloc(sizeof(Rationnel));

   rat->etiquette = etiquette;
   rat->lettre = lettre;
   rat->position_min = position_min;
   rat->position_max = position_max;
   rat->data = data;
   rat->gauche = gauche;
   rat->droit = droit;
   rat->pere = pere;
   return rat;
}



Rationnel *Epsilon()
{
   return rationnel(EPSILON, 0, 0, 0, NULL, NULL, NULL, NULL);
}

Rationnel *Lettre(char l)
{
   return rationnel(LETTRE, l, 0, 0, NULL, NULL, NULL, NULL);
}

Rationnel *Union(Rationnel* rat1, Rationnel* rat2)
{
   // Cas particulier où rat1 est vide
   if (!rat1)
      return rat2;

   // Cas particulier où rat2 est vide
   if (!rat2)
      return rat1;
   
   return rationnel(UNION, 0, 0, 0, NULL, rat1, rat2, NULL);
}

Rationnel *Concat(Rationnel* rat1, Rationnel* rat2)
{
   if (!rat1 || !rat2)
      return NULL;

   if (get_etiquette(rat1) == EPSILON)
      return rat2;

   if (get_etiquette(rat2) == EPSILON)
      return rat1;
   
   return rationnel(CONCAT, 0, 0, 0, NULL, rat1, rat2, NULL);
}

Rationnel *Star(Rationnel* rat)
{
   return rationnel(STAR, 0, 0, 0, NULL, rat, NULL, NULL);
}



bool est_racine(Rationnel* rat)
{
   return (rat->pere == NULL);
}

Noeud get_etiquette(Rationnel* rat)
{
   return rat->etiquette;
}

char get_lettre(Rationnel* rat)
{
   return rat->lettre;
}

int get_position_min(Rationnel* rat)
{
   return rat->position_min;
}

int get_position_max(Rationnel* rat)
{
   return rat->position_max;
}


void set_position_min(Rationnel* rat, int valeur)
{
   rat->position_min = valeur;
   return;
}

void set_position_max(Rationnel* rat, int valeur)
{
   rat->position_max = valeur;
   return;
}


Rationnel *fils_gauche(Rationnel* rat)
{
   return rat->gauche;
}

Rationnel *fils_droit(Rationnel* rat)
{
   return rat->droit;
}

Rationnel *fils(Rationnel* rat)
{
   return rat->gauche;
}

Rationnel *pere(Rationnel* rat)
{
   return rat->pere;
}


void print_rationnel(Rationnel* rat)
{
   if (rat == NULL)
   {
      printf("∅");
      return;
   }
   
   switch(get_etiquette(rat))
   {
      case EPSILON:
         printf("ε");         
         break;
         
      case LETTRE:
         printf("%c", get_lettre(rat));
         break;

      case UNION:
         printf("(");
         print_rationnel(fils_gauche(rat));
         printf(" + ");
         print_rationnel(fils_droit(rat));
         printf(")");         
         break;

      case CONCAT:
         printf("[");
         print_rationnel(fils_gauche(rat));
         printf(" . ");
         print_rationnel(fils_droit(rat));
         printf("]");         
         break;

      case STAR:
         printf("{");
         print_rationnel(fils(rat));
         printf("}*");         
         break;

      default:
         break;
   }
}


Rationnel *expression_to_rationnel(const char *expr)
{
    Rationnel *rat;
    yyscan_t scanner;
    YY_BUFFER_STATE state;

    // Initialisation du scanner
    if (yylex_init(&scanner))
        return NULL;
 
    state = yy_scan_string(expr, scanner);

    // Test si parsing ok.
    if (yyparse(&rat, scanner)) 
        return NULL;
    
    // Libération mémoire
    yy_delete_buffer(state, scanner);
 
    yylex_destroy(scanner);
 
    return rat;
}

void rationnel_to_dot(Rationnel *rat, char* nom_fichier)
{
   FILE *fp = fopen(nom_fichier, "w+");
   rationnel_to_dot_aux(rat, fp, -1, 1);
}

int rationnel_to_dot_aux(Rationnel *rat, FILE *output, int pere, int noeud_courant)
{   
   int saved_pere = noeud_courant;

   if (pere >= 1)
      fprintf(output, "\tnode%d -> node%d;\n", pere, noeud_courant);
   else
      fprintf(output, "digraph G{\n");
   
   switch(get_etiquette(rat))
   {
      case LETTRE:
         fprintf(output, "\tnode%d [label = \"%c-%d\"];\n", noeud_courant, get_lettre(rat), rat->position_min);
         noeud_courant++;
         break;

      case EPSILON:
         fprintf(output, "\tnode%d [label = \"ε-%d\"];\n", noeud_courant, rat->position_min);
         noeud_courant++;
         break;

      case UNION:
         fprintf(output, "\tnode%d [label = \"+ (%d/%d)\"];\n", noeud_courant, rat->position_min, rat->position_max);
         noeud_courant = rationnel_to_dot_aux(fils_gauche(rat), output, noeud_courant, noeud_courant+1);
         noeud_courant = rationnel_to_dot_aux(fils_droit(rat), output, saved_pere, noeud_courant+1);
         break;

      case CONCAT:
         fprintf(output, "\tnode%d [label = \". (%d/%d)\"];\n", noeud_courant, rat->position_min, rat->position_max);
         noeud_courant = rationnel_to_dot_aux(fils_gauche(rat), output, noeud_courant, noeud_courant+1);
         noeud_courant = rationnel_to_dot_aux(fils_droit(rat), output, saved_pere, noeud_courant+1);
         break;

      case STAR:
         fprintf(output, "\tnode%d [label = \"* (%d/%d)\"];\n", noeud_courant, rat->position_min, rat->position_max);
         noeud_courant = rationnel_to_dot_aux(fils(rat), output, noeud_courant, noeud_courant+1);
         break;
         
      default:
         break;
   }
   if (pere < 0)
      fprintf(output, "}\n");
   return noeud_courant;
}


/*/
 * Numéroter rationnel :
 * Chaque lettre enregistre sa position dans l'expression rationnelle.
 * Chaque sommet contient la position minimale et maximale des lettres contenues dans ses fils.
/*/

int numeroter_rationnel_aux (Rationnel* noeud, int m){
   // Cette fonction renvoie le nombre de lettres de l'expression +1
   // Plus pratique pour numeroter les lettres et récupérer leur nombre.

   switch(get_etiquette(noeud)){

      case EPSILON :
      // On ne le considère pas comme une lettre, donc pas d'incrémentation,
      // cependant sa position est, par conséquent, la meme que la lettre suivante.
      // Cela reste un pur choix de notre part, nous aurions pu mettre "0", par exemple.
         set_position_min(noeud, m);
         set_position_max(noeud, m);
         return m;
         break;

      case LETTRE :
      // On "set" les deux positions, simple précaution, on pourrait en choisir qu'une
         set_position_min(noeud, m);
         set_position_max(noeud, m);
         m++;
         return m;
         break;

      case UNION :
      case CONCAT :
      // Petit mix de parcours préfixe/postfixe. Le deuxième "set" est de "m-1" car
      // on incrémente par défaut après chaque lettre. Avec cela, on a, à chaque noeud :
      // get_position_min(noeud) <= position d'une lettre <= get_position_max(noeud)
         set_position_min(noeud, m);
         m = numeroter_rationnel_aux(fils_gauche(noeud), m);
         m = numeroter_rationnel_aux(fils_droit(noeud), m);
         set_position_max(noeud, m-1);
         return m;
         break;

      case STAR :
      // Simple cas particulier du précédent (pas de fils droit).
         set_position_min(noeud, m);
         m = numeroter_rationnel_aux(fils_gauche(noeud), m);
         set_position_max(noeud, m-1);
         return m;
         break;

      default:
         return 0;

   }

}

void numeroter_rationnel (Rationnel* racine){
   if (racine!=NULL)
      numeroter_rationnel_aux(racine, 1);
}


/*/
 * Contient mot vide :
 * Renvoie "true" si le langage exprimé par l'expression rationnelle en question peut
 * contenir le mot vide.
/*/

bool contient_mot_vide(Rationnel *rat){

   switch(get_etiquette(rat)){

      case EPSILON:
      // Renvoie "true" car si un mot ne contient qu'Epsilon, c'est le mot vide.
         return true;
         break;

      case LETTRE:
      // Une lettre renvoie "false", elle ne peut valoir le mot vide.
         return false;
         break;

      case STAR:
      // Renvoie "true" car une expression étoilée peut valoir le mot vide, peu
      // importe son fils.
         return true;
         break;

      // Les deux suivants ne sont ensuite que composition des précédents.

      case CONCAT:
      // Les deux doivent pouvoir etre vides pour pouvoir renvoyer "true"...
         return contient_mot_vide(fils_gauche(rat)) && contient_mot_vide(fils_droit(rat));
         break;

      case UNION:
      // ... et ici un seul des deux suffit.
         return contient_mot_vide(fils_gauche(rat)) || contient_mot_vide(fils_droit(rat));
         break;

      default:
         return NULL;

   }

}

/* Met à jour le champ "pere" de tous les noeuds du Rationnel 
* (corrige le bug ne le mettant pas à jour durant l'édition d'un Rationnel) */

void pere_a_jour(Rationnel * rat, Rationnel * last)
{
   rat->pere = last;

   if (fils_gauche(rat) != NULL) //Si le fils gauche existe, on met à jour le père du fil gauche
   {
      pere_a_jour(fils_gauche(rat), rat);
   }

   if (fils_droit(rat) != NULL)// De même pour le fils droit
   {
      pere_a_jour(fils_droit(rat), rat);
   }
}

/* Ajoute dans l'Ensemble toutes les lettres pouvant se trouver à la première place 
* d'un mot décrit par le Rationnel. */

bool premier_aux (Rationnel * rat, Ensemble * ens){
   switch(get_etiquette(rat)){
      case EPSILON:
      //Si c'est le mot vide, on ne fait rien, donc on renvoie false
      //pour dire que rien n'a été ajouté
         return false;
         break;
      case LETTRE:
      //Si c'est une lettre, on renvoie true car on a ajouté un élément 
      //dans l'ensemble
         ajouter_element(ens, get_position_min(rat));
         return true;
         break;
      case STAR:
      //SI c'est une étoile, on cherche les premiers du fils gauche
      //mais on renvoie false, car on n'ajoute pas d'élément
         premier_aux(fils(rat), ens);
         return false;
         break;
      case UNION:
      //Si c'est une union et que des éléments ont été ajoutés dans
      //le fils gauche, on regarde le sous arbre fils droit, sinon
      //on ne regarde que le fils droit
         if (premier_aux(fils_gauche(rat), ens)) {
            premier_aux(fils_droit(rat), ens);
            return true;
         }
         return premier_aux(fils_droit(rat), ens);
         break;
      case CONCAT:
      //Si c'est une concaténation, si des éléments ont été ajoutés
      //dans le fils gauche, on renvoie true et on ne regarde pas
      //le fils droit, sinon on regarde le fils droit
         if (contient_mot_vide(fils_gauche(rat)))
         {
            premier_aux(fils_gauche(rat), ens);
            premier_aux(fils_droit(rat), ens);

            return true;
         }
         else
            return premier_aux(fils_gauche(rat), ens);
         break;
      default:
         return NULL;
   }

   return NULL;
}


Ensemble * premier(Rationnel *rat)
{
   Ensemble * e = creer_ensemble(NULL, NULL, NULL);

   pere_a_jour(rat, NULL);
   premier_aux(rat, e);

   return e; 
}

/** Retourne le langage "miroir", c'est-à-dire celui qui reconnaît tous les mots 
* reconnus par le langage mais à l'envers. 
*(un palindrome est reconnu par les deux langages, du coup) */
Rationnel *miroir_expression_rationnelle(Rationnel *rat)
{
   if(!rat)
      return NULL;

   Rationnel *f1, *f2;
   switch(get_etiquette(rat)){
      case EPSILON:
         return Epsilon();
         break;
      case LETTRE:
         f1 = Lettre(get_lettre(rat));
         set_position_min(f1, get_position_min(rat));
         set_position_max(f1, get_position_max(rat));
         return f1;
         break;
      case UNION:
         f1 = miroir_expression_rationnelle(fils_gauche(rat));
         f2 = miroir_expression_rationnelle(fils_droit(rat));
         return Union(f1, f2);
         break;
      case CONCAT:
         f1 = miroir_expression_rationnelle(fils_gauche(rat));
         f2 = miroir_expression_rationnelle(fils_droit(rat));
         return Concat(f2, f1);
         break;
      case STAR:
         f1 = miroir_expression_rationnelle(fils(rat));
         return Star(f1);
         break;
      default:
         break;
   }

   return NULL;
}

Ensemble *dernier(Rationnel *rat){
   /* L'ensemble des derniers n'est, au final, que l'ensemble des premiers 
   * de l'expression miroir */
   Rationnel * r = miroir_expression_rationnelle(rat);
   print_rationnel(r);
   return premier(r);
}

/* Retrouve le Rationnel correspondant à la position donnée. */
Rationnel* find_position(Rationnel* rat, int position){
   if (get_etiquette(rat) == LETTRE && get_position_min(rat) == position)
   {
      return rat;
   }

   if (fils_droit(rat) != NULL && get_position_min(fils_droit(rat)) <= position)
   {
      return find_position(fils_droit(rat), position);
   }

   if (fils_gauche(rat) != NULL)
   {
      return find_position(fils_gauche(rat), position);
   }

   return NULL;

}

/* Ajoute dans l'Ensemble toutes les lettres pouvant suivre la lettre donnée. */
Ensemble *suivant(Rationnel *rat, int position)
{
 
   Ensemble * ens = creer_ensemble(NULL, NULL, NULL);

   pere_a_jour(rat, NULL);
   Rationnel * r = find_position(rat, position);
   Rationnel * last;


   while (r!=NULL){

      switch (get_etiquette(r)){
         case LETTRE:
         case EPSILON:
         case UNION:
            break;
         case CONCAT:
            if (last == fils_gauche(r))
            { 
               premier_aux(fils_droit(r), ens);
               if (!contient_mot_vide(fils_droit(r)))
                  r = NULL;
            }
            
            break;
         case STAR:
            premier_aux(r, ens);
            break;
         default:
            return NULL;
      }
      last = r;
      if (r != NULL)
         r = r->pere;
      
   }

   return ens;
}

/* Retourne la lettre correspondant à la position donnée. 
* (composée simplifiée de get_lettre(find_position(rat, position)) */
char get_position_lettre(Rationnel * rat, int position)
{
   switch(get_etiquette(rat))
   {
      case EPSILON:
         break;
      case LETTRE:
         if (get_position_min(rat) == position)
            return rat->lettre;
         break;
      case STAR:
         return get_position_lettre(fils(rat), position);
         break;
      case UNION:
      case CONCAT:
         if (position <= get_position_max(fils_gauche(rat)))
            return get_position_lettre(fils_gauche(rat), position);
         else
            return get_position_lettre(fils_droit(rat), position);
         break;
   }

   return 0;
}

/* Retourne un automate décrivant le langage décrit par le Rationnel donné. */
Automate *Glushkov(Rationnel *rat)
{
   Automate* aut = creer_automate();

   Ensemble * ens = premier(rat);
   Ensemble_iterateur it;
   ajouter_etat_initial(aut, 0);
   for (it = premier_iterateur_ensemble(ens);
      iterateur_ensemble_est_vide(it) != 1;
      it = iterateur_suivant_ensemble(it)) 
   {
      ajouter_etat(aut, get_element(it));
      ajouter_transition(aut, 0, get_position_lettre(rat, get_element(it)), get_element(it));
      
   }

   if (contient_mot_vide(rat))
      ajouter_etat_final(aut, 0);

   const Ensemble * etat = get_etats(aut);

 
   for (it = premier_iterateur_ensemble(etat);
      iterateur_ensemble_est_vide(it) != 1;
      it = iterateur_suivant_ensemble(it))
   {
      Ensemble * next = suivant(rat, get_element(it));
      Ensemble_iterateur it1;
      for (it1 = premier_iterateur_ensemble(next);
         iterateur_ensemble_est_vide(it1) != 1;
         it1 = iterateur_suivant_ensemble(it1))
      {
         ajouter_transition(aut, get_element(it), get_position_lettre(rat, get_element(it1)), get_element(it1));

      }
  
   }

   vider_ensemble(ens);
   ens = dernier(rat);

   for ( it = premier_iterateur_ensemble(ens);
      iterateur_ensemble_est_vide(it) != 1;
      it = iterateur_suivant_ensemble(it)) 
   {
      ajouter_etat_final(aut, get_element(it));
   }

   return aut;

}

/* Fonction appelée par pour_toute_transition.
* Teste si une transition d'un automate se trouve dans un
* autre automate */
void testerAutomateDans(int origine, char lettre, int fin, void * data)
{ 
   Data * a = (Data *) data; 
   if (!est_une_transition_de_l_automate(a->a, origine, lettre, fin))
   { 
      a->estDansSecondAutomate = false; 
   } 
}

/* Vérifie que deux automates quelconques vérifient le même langage. */
bool automates_reconnaissent_le_meme_langage (Automate * aut1, Automate * aut2)
{
   aut1 = creer_automate_minimal(aut1);
   aut2 = creer_automate_minimal(aut2);


     if (comparer_ensemble(aut1->vide, aut2->vide) == 0) 
     {
      if (comparer_ensemble(aut1->etats, aut2->etats) == 0) 
      {
         if (comparer_ensemble(aut1->initiaux, aut2->initiaux) == 0) 
         {
            if (comparer_ensemble(aut1->finaux, aut2->finaux) == 0)
            {
               Data * d = malloc(sizeof(Data)); 
               d->estDansSecondAutomate = true; 
               d->a = aut2; 
            
               pour_toute_transition(aut1, testerAutomateDans, d);

               Data * d1 = malloc(sizeof(Data)); 
               d1->estDansSecondAutomate = true; 
               d1->a = aut1;
               pour_toute_transition(aut2, testerAutomateDans, d1); 

               return d->estDansSecondAutomate && d1->estDansSecondAutomate;
            }
         }
      }
   }

   return false;
}

/* Vérifie que deux expressions rationnelles textuelles (char*) reconnaissent 
* le même langage. */
bool meme_langage (const char *expr1, const char* expr2)
{

   /*expr ---(expr_to_rationnel)--> Rationnel* ---(Glushkov)--> Automate*
   * On minimalise les automates, et on les compare. */
   Rationnel* rat1, *rat2;
   rat1 = expression_to_rationnel(expr1);
   rat2 = expression_to_rationnel(expr2);

   numeroter_rationnel(rat1);
   numeroter_rationnel(rat2);

   Automate * aut1, *aut2;
   aut1 = Glushkov(rat1);
   aut2 = Glushkov(rat2);

  
   return automates_reconnaissent_le_meme_langage(aut1, aut2);

}

void remplir_matrice(int origine, char lettre, int fin, void * data)
{
   Systeme sys = (Systeme) data;
   sys[origine][fin] = Lettre(lettre);
}

void ajouter_epsilon(const intptr_t element, void * data)
{
   Datasys * d = (Datasys *) data;
   Systeme sys = d->sys;
   int etat = (int) element;

   int colonne = d->taille;
   sys[etat][colonne] = Epsilon();
}


Systeme systeme(Automate *automate)
{
   const Ensemble * ens = get_etats(automate);
   int taille = taille_ensemble(ens);
   Systeme sys = malloc(taille * sizeof(Rationnel **));

   for (int ind = 0; ind < taille; ind++)
      sys[ind] = malloc((taille + 1) * sizeof(Rationnel *));

   pour_toute_transition(automate, remplir_matrice, sys);
   ens = get_finaux(automate);

   Datasys * data = malloc (sizeof(Datasys));
   data->taille = taille;
   data->sys = sys;

   pour_tout_element(ens, ajouter_epsilon, data);

   return sys;
}


void print_ligne(Rationnel **ligne, int n)
{
   for (int j = 0; j <=n; j++)
      {
         print_rationnel(ligne[j]);
         if (j<n)
            printf("X%d\t+\t", j);
      }
   printf("\n");
}

void print_systeme(Systeme systeme, int n)
{
   for (int i = 0; i <= n-1; i++)
   {
      printf("X%d\t= ", i);
      print_ligne(systeme[i], n);
   }
}


Rationnel **resoudre_variable_arden(Rationnel **ligne, int numero_variable, int nb_vars)
{
   if (ligne[numero_variable] == NULL)
      return ligne;

   for (int i = 0; i <= nb_vars; i++)
   {
      if (ligne[i] != NULL && i != numero_variable)
         ligne[i] = Concat(Star(ligne[numero_variable]), ligne[i]);
   }

   ligne[numero_variable] = NULL;
   return ligne;
}

Rationnel **substituer_variable(Rationnel **ligne, int numero_variable, Rationnel **valeur_variable, int n)
{
   if (ligne[numero_variable] == NULL)
      return ligne;

   for (int i = 0; i <= n; i++)
   {
      if (valeur_variable[i] != NULL && i != numero_variable)
      {
         if (ligne[i] == NULL)
            ligne[i] = Concat(ligne[numero_variable], valeur_variable[i]);
         else
            ligne[i] = Union(ligne[i], Concat(ligne[numero_variable], valeur_variable[i]));
      }
   }

   ligne[numero_variable] = NULL;
   return ligne;
}  


Systeme resoudre_systeme(Systeme systeme, int nb_vars)
{
   /* Les deux fonctions gèrent elles-mêmes tous les cas.
   *Dans le cas où on n'a pas besoin de les appeler, elles
   *ne feront rien. */
   for (int i = nb_vars - 1; i >= 0; i--)
   {
      systeme[i] = resoudre_variable_arden(systeme[i], i, nb_vars);

      for (int j = 0; j < i; j++)
         systeme[j] = substituer_variable(systeme[j], i, systeme[i], nb_vars);
   }

   return systeme;
}

/* Retourne un Rationnel décrivant l'automate donné. */
Rationnel *Arden(Automate *automate)
{
   // Crée une matrice représentant le système d'équations de l'automate
   Systeme sys = systeme(automate);
   int taille = taille_ensemble(get_etats(automate));
   // Résoud le système et sauvegarde le résultat dans sys[0][taille]
   sys = resoudre_systeme(sys, taille);

   return sys[0][taille];
}
