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

// Enlever les @note

int yyparse(Rationnel **rationnel, yyscan_t scanner);

// CONSTRUCTEURS @note

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

// SETTERS @note

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

// GETTERS @note

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
   assert (get_etiquette(rat) == LETTRE);
   return rat->lettre;
}

int get_position_min(Rationnel* rat)
{
   assert (get_etiquette(rat) == LETTRE);
   return rat->position_min;
}

int get_position_max(Rationnel* rat)
{
   assert (get_etiquette(rat) == LETTRE);
   return rat->position_max;
}

// SETTERS @note

void set_position_min(Rationnel* rat, int valeur)
{
   assert (get_etiquette(rat) == LETTRE);
   rat->position_min = valeur;
   return;
}

void set_position_max(Rationnel* rat, int valeur)
{
   assert (get_etiquette(rat) == LETTRE);
   rat->position_max = valeur;
   return;
}

// GETTERS @note

Rationnel *fils_gauche(Rationnel* rat)
{
   assert((get_etiquette(rat) == CONCAT) || (get_etiquette(rat) == UNION));
   return rat->gauche;
}

Rationnel *fils_droit(Rationnel* rat)
{
   assert((get_etiquette(rat) == CONCAT) || (get_etiquette(rat) == UNION));
   return rat->droit;
}

Rationnel *fils(Rationnel* rat)
{
   assert(get_etiquette(rat) == STAR);
   return rat->gauche;
}

Rationnel *pere(Rationnel* rat)
{
   assert(!est_racine(rat));
   return rat->pere;
}

// TOSTRING @note

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
         assert(false);
         break;
   }
}

// PARSING & OUTFILE @note

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
         assert(false);
         break;
   }
   if (pere < 0)
      fprintf(output, "}\n");
   return noeud_courant;
}

// TODO 1ST PART @note
// Doit-on faire avec les accesseurs ou avec les champs de la structure ? On y a accès depuis ici après tout. @note

int numeroter_rationnel_aux (Rationnel* noeud, int m){
   switch(get_etiquette(noeud)){
      case EPSILON :
         return m;
      case LETTRE :
         noeud->position_min = (noeud->position_max = m);
         m++;
         return m;
      case UNION || CONCAT :
         noeud->position_min = m;
         m = numeroter_rationnel_aux(noeud->gauche, m);
         m = numeroter_rationnel_aux(noeud->droite, m);
         noeud->position_max = m-1;
         return m;
      case STAR :
         noeud->position_min = m;
         m = numeroter_rationnel_aux (noeud->gauche, m);
         if (!noeud->droite=NULL)
            m = numeroter_rationnel_aux(noeud->droite, m);
         noeud->position_max = m-1;
         return m;
      default:
         return 0;
   }
}

void numeroter_rationnel (Rationnel* racine){
   if (!racine)
      numeroter_rationnel_aux(racine, 1);
}

// Sous-fonctions de Glushkov @note

bool contient_mot_vide(Rationnel *rat)
{
   switch(get_etiquette(rat)){
      case EPSILON:
         return true;
         break;
      case LETTRE:
         return false;
         break;
      case CONCAT:
         return contient_mot_vide(fils_gauche(rat))&&contient_mot_vide(fils_droit(rat));
         break;
      case UNION:
         return contient_mot_vide(fils_gauche(rat))||contient_mot_vide(fils_droit(rat));
         break;
      case STAR:
         return true;
         break;
      default:
         return NULL;
   }
}

// ATTENTION ! IL FAUDRA UTILISER LA STRUCTURE "ENSEMBLE" A TERME @note
// Pourquoi pas écrire une fonction transformant un [tab + ind] en Ensemble ? Plus simple, on a le nb d'éléments dans ind

bool premier_aux (Rationnel * rat, int* tab, int ind){
   switch(get_etiquette(rat)){
      case EPSILON:
         return false;
         break;
      case LETTRE:
         tab[*ind] = get_position_min(rat);
         *ind++;
         return true;
         break;
      case STAR:
         return false;
         break;
      case UNION:
         return premier_aux(fils_gauche(rat)) || premier_aux(fils_droit(rat));
         break;
      case CONCAT:
         if (premier_aux(fils_gauche(rat)))
            return true;
         premier_aux(fils_droit(rat));
         break;
      default:
         return;
   }
}

// Mauvaise valeur de retour, voir plus haut @note

Ensemble * premier(Rationnel *rat)
{
   int n = numeroter_rationnel_aux(rat, 0) -1; // Vaut par conséquent le nombre de lettres de l'expression rationnelle
   int* tab = malloc(n*sizeof(int));
   n = 0; // Economie de variables
   premier_aux(rat, tab, n);
   return tab; 
}

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
         return Lettre(get_lettre(rat));
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
         assert(false);
         break;
   }
}

Ensemble *dernier(Rationnel *rat)
{
   Rationnel r = miroir_expression_rationnelle(rat);
   return premier(r);
}

// Pour "suivant" @note
// Parcours itératif de l'arbre, et à chaque noeud "LETTRE" on teste la position, on return si c'est la bonne @note
// Autre solution plus optimale, utiliser les position_min et position_max des noeuds @note

Rationnel* find_position(Rationnel* rat, int position){

   Rationnel* last = NULL;
   Rationnel* next = NULL;

   while (rat != NULL){

      if (get_etiquette(rat) == LETTRE && get_position_min(rat) == position) // Si c'est une lettre et que c'est la position recherchée
         return rat;

      if (last == pere(rat))
      {
         last = rat;
         next = fils_gauche(rat);
      }
      
      if (next == NULL || last == fils_gauche(rat))
      {
         last = rat;
         next = fils_droit(rat);
      }
      
      if (next == NULL || last == fils_droit(rat))
      {
         last = rat;
         next = pere(rat);
      }

      rat = next;

   }

   return NULL; // Précaution

}

// Pseudo-code pour l'instant. Avant de la finaliser, peut-^etre s'intéresser à la structure Ensemble. @note

Ensemble *suivant(Rationnel *rat, int position)
{
   /*
   ALGORITHME :
   parcours jusqu'à la position
   remonter l'arbre :
   - si UNION :
      remonter // Dans (a+b), on se fiche de laquelle a été mise, l'autre ne le sera pas
   - si CONCAT :
      - si FG :
         premier_aux(FD)
      remonter // Dans (a.b), si on a mis a (FG), on doit mettre b (globalement, les "premier" du FD), sinon rien à mettre
   - si EPSILON :
      remonter
   - si STAR :
      premier_aux(self) // Dans R*, quelle que soit R, on peut mettre les "premier" de R
      remonter
   - si LETTRE :
      normalement impossible puisqu'on ne fait que remonter
   une fois à la racine, on renvoie.
   */

   int n = numeroter_rationnel_aux(rat, 0) -1; // Voir avec les Ensemble, ne pas oublier @note
   int* tab = malloc(n*sizeof(int));
   n = 0;

   rat = find_position(rat, position);

   while (rat!=NULL){

      switch (get_etiquette(rat)){
         case EPSILON:
            rat = pere(rat);
            break;
         case UNION:
            rat = pere(rat);
            break;
         case CONCAT:
            if (rat == fils_gauche(pere(rat)))
               premier_aux(fils_droit(pere(rat)), tab, n);
            rat = pere(rat);
            break;
         case STAR:
            premier_aux(rat, tab, n);
            break;
         default:
            return;
      }
      
   }

   return tab;
}

// Glushkov @note

Automate *Glushkov(Rationnel *rat)
{
   A_FAIRE_RETURN(NULL);
}

// La véritable raison (de vivre) de Glushkov @note

bool meme_langage (const char *expr1, const char* expr2)
{
   // expr ---(expr_to_rationnel)--> Rationnel* ---(Glushkov)--> Automate*
   // On minimalise les automates, et on les compare
   A_FAIRE_RETURN(true);
}

// TODO 2ND PART @note

Systeme systeme(Automate *automate)
{
   A_FAIRE_RETURN(NULL);
}

// PRINTS @note

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

// TODO 2ND PART @note

Rationnel **resoudre_variable_arden(Rationnel **ligne, int numero_variable, int n)
{
   A_FAIRE_RETURN(NULL);
}

Rationnel **substituer_variable(Rationnel **ligne, int numero_variable, Rationnel **valeur_variable, int n)
{
   A_FAIRE_RETURN(NULL);
}

Systeme resoudre_systeme(Systeme systeme, int n)
{
   A_FAIRE_RETURN(NULL);
}

Rationnel *Arden(Automate *automate)
{
   A_FAIRE_RETURN(NULL);
}
