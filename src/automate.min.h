#ifndef __AUTOMATE_H__
#define __AUTOMATE_H__

#include "ensemble.h"

struct Automate {
   	Ensemble * vide;
	Ensemble * etats;
	Ensemble * alphabet;
	Table* transitions;
	Ensemble * initiaux;
	Ensemble * finaux;
};

typedef struct Automate Automate;

typedef struct Cle {
	int origine;
	int lettre;
} Cle;

Automate * creer_automate();
void liberer_automate( Automate * automate);

void ajouter_etat( Automate * automate, int etat );
void ajouter_lettre( Automate * automate, char lettre );
void ajouter_transition( Automate * automate, int origine, char lettre, int fin );
void ajouter_etat_final( Automate * automate, int etat_final );
void ajouter_etat_initial( Automate * automate, int etat_initial );

const Ensemble * get_etats( const Automate* automate );
const Ensemble * get_initiaux( const Automate* automate );
const Ensemble * get_finaux( const Automate* automate );
const Ensemble * get_alphabet( const Automate* automate );

int est_une_transition_de_l_automate( const Automate* automate,	int origine, char lettre, int fin );
int est_un_etat_de_l_automate( const Automate* automate, int etat );
int est_un_etat_initial_de_l_automate( const Automate* automate, int etat );
int est_un_etat_final_de_l_automate( const Automate* automate, int etat );
int est_une_lettre_de_l_automate( const Automate* automate, char lettre );

Ensemble * delta( const Automate* automate, const Ensemble * etats_courants, char lettre );
Ensemble * delta_star( const Automate* automate, const Ensemble * etats_courants, const char* mot );
int le_mot_est_reconnu( const Automate* automate, const char* mot );
void pour_toute_transition( const Automate* automate, void (* action )( int origine, char lettre, int fin, void* data ), void* data );
int get_max_etat( const Automate* automate );
int get_min_etat( const Automate* automate );
Automate* copier_automate( const Automate* automate );
Ensemble* etats_accessibles( const Automate * automate, int etat );
Ensemble* accessibles( const Automate * automate );
Automate *automate_accessible( const Automate * automate );
Automate *miroir( const Automate * automate);
void print_automate( const Automate * automate );
Automate * creer_intersection_des_automates( const Automate * automate_1, const Automate * automate_2 );
Automate * creer_automate_deterministe( const Automate* automate );
Automate * creer_automate_minimal( const Automate* automate );
int nombre_de_transitions( const Automate* automate );

#endif
