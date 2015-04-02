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

// SETTERS @note

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

// GETTERS @note

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
         break;
   }
   if (pere < 0)
      fprintf(output, "}\n");
   return noeud_courant;
}

// TODO 1ST PART @note

/*/
 * Numéroter rationnel :
 * Chaque lettre enregistre sa position dans l'expression rationnelle.
 * Chaque sommet contient la position minimale et maximale des lettres contenues dans ses fils.
/*/

int numeroter_rationnel_aux (Rationnel* noeud, int m){
   // Cette fonction renvoie le nombre de lettres de l'expression +1
   // Pratique pour numeroter les lettres et récupérer leur nombre.

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

// Sous-fonctions de Glushkov @note

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

// ATTENTION ! IL FAUDRA UTILISER LA STRUCTURE "ENSEMBLE" A TERME @note
// Pourquoi pas écrire une fonction transformant un [tab + ind] en Ensemble ? Plus simple, on a le nb d'éléments dans ind @note

bool premier_aux (Rationnel * rat, Ensemble * ens){
   switch(get_etiquette(rat)){
      case EPSILON:
         return false;
         break;
      case LETTRE:
         ajouter_element(ens, get_position_min(rat));
         return true;
         break;
      case STAR:
         premier_aux(fils_gauche(rat), ens);
         return false;
         break;
      case UNION:
         if (premier_aux(fils_gauche(rat), ens)) {
            premier_aux(fils_droit(rat), ens);
            return true;
         }
         return premier_aux(fils_droit(rat), ens);
         break;
      case CONCAT:
         if (premier_aux(fils_gauche(rat), ens))
            return true;
         premier_aux(fils_droit(rat), ens);
         break;
      default:
         return NULL;
   }

   return NULL;
}

// Mauvaise valeur de retour, voir plus haut @note

Ensemble * premier(Rationnel *rat)
{
   Ensemble * e = creer_ensemble(NULL, NULL, NULL);

   premier_aux(rat, e);

   return e; 
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
         break;
   }

   return NULL;
}

Ensemble *dernier(Rationnel *rat){
   // L'ensemble des derniers n'est, au final, que l'ensemble des premiers de l'expression miroir...
   Rationnel * r = miroir_expression_rationnelle(rat);
   return premier(r);
}

// Pour "suivant" @note
// Parcours itératif de l'arbre, et à chaque noeud "LETTRE" on teste la position, on return si c'est la bonne @note
// Autre solution plus optimale, utiliser les position_min et position_max des noeuds @note

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

void pere_a_jour(Rationnel * rat, Rationnel * last)
{
   rat->pere = last;

   if (fils_gauche(rat) != NULL)
   {
      pere_a_jour(fils_gauche(rat), rat);
   }

   if (fils_droit(rat) != NULL)
   {
      pere_a_jour(fils_droit(rat), rat);
   }
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

   Ensemble * ens = creer_ensemble(NULL, NULL, NULL);

   Rationnel * r = find_position(rat, position);
   Rationnel * last;

   pere_a_jour(rat, NULL);
   while (r!=NULL){

      switch (get_etiquette(r)){
         case LETTRE:
         case EPSILON:
         case UNION:
            break;
         case CONCAT:
            if (last == fils_gauche(r))
               premier_aux(fils_droit(r), ens);
            break;
         case STAR:
            premier_aux(r, ens);
            break;
         default:
            return NULL;
      }
      last = r;
      r = r->pere;
      
   }

   return ens;
}

// Glushkov @note

/* Fonctions pour Glushkov @note
Automate * creer_automate();
void liberer_automate( Automate * automate);

void ajouter_etat( Automate * automate, int etat );
void ajouter_lettre( Automate * automate, char lettre );
void ajouter_transition( Automate * automate, int origine, char lettre, int fin );
void ajouter_etat_final( Automate * automate, int etat_final );
void ajouter_etat_initial( Automate * automate, int etat_initial );
*/

Automate *Glushkov(Rationnel *rat)
{
   Automate* aut = creer_automate();

   Ensemble * ens = premier(rat);
   Ensemble_iterateur it = premier_iterateur_ensemble(ens);
   while(iterateur_ensemble_est_vide(it)!=1) {
      ajouter_etat_initial(aut, get_element(it));
      it = iterateur_suivant_ensemble(it);
   }
   
   ens = dernier(rat);
   it = premier_iterateur_ensemble(ens);
   while(iterateur_ensemble_est_vide(it)!=1) {
      ajouter_etat_final(aut, get_element(it));
      it = iterateur_suivant_ensemble(it);
   }

   
   return NULL;







}

// La véritable raison (de vivre) de Glushkov @note

bool meme_langage (const char *expr1, const char* expr2)
{
   // expr ---(expr_to_rationnel)--> Rationnel* ---(Glushkov)--> Automate*
   // On minimalise les automates, et on les compare
   Rationnel* rat1, *rat2;
   rat1 = expression_to_rationnel(expr1);
   rat2 = expression_to_rationnel(expr2);

   Automate * aut1, *aut2;
   aut1 = Glushkov(rat1);
   aut2 = Glushkov(rat2);

   aut1 = creer_automate_minimal(aut1);
   aut2 = creer_automate_minimal(aut2);

   // Reste à les comparer

   A_FAIRE_RETURN(true);
}

// <!--

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

// -->