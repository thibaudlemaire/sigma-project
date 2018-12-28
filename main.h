////////////////////////////////////////////////////////////////
////														////
////			P P E  S - S I    2011-2012					////
////				Projet S I G M A						////
////		   Lycée Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////						main.h							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de déclarations de main.c		////
////														////
////	Créé le : 05/12/2011								////
////	Modifié le : 05/12/2011								////
////	Support : PIC 18F2525  -  16 MHz					////
////	Par : 	Thibaud LEMAIRE								////
////														////
////////////////////////////////////////////////////////////////

#ifndef main_h
#define main_h

//// Defines ////
#define TEMPO_ERREUR 30
#define TEMPO_REVEIL 5
#define TEMPO_NOMS 20
#define TEMPO_INIT 5
#define TEMPO_MESSAGE 40
#define ERREUR_SAISIE 1
#define ERREUR_UP 2
#define ERREUR_IMPOSSIBLE 3

//// Définitions ////
extern char consigne_tilt;			// Consigne du tilt
extern char consigne_roll;			// Consigne du roll
extern struct
{
	unsigned choix_mode				:1;		// Boucle de choix du mode
	unsigned choix_mode_2			:1;		// Boucle de choix du mode de rang 2, appuis sur les touches
	unsigned auto_connexion			:1;		// Mode auto, message de connexion
	unsigned auto_connecte			:1;		// Mode auto, connecté
	unsigned manu_choix_axe			:1;		// Mode manuel, choix de l'axe par l'utilisateur
	unsigned manu_choix_axe_2		:1;		// Mode manuel, chois de l'axe, niveau 2
	unsigned manu_tilt				:1;		// Mode manuel, modification du Tilt
	unsigned manu_roll				:1; 	// Mode manuel, modification du Roll
	unsigned demo					:1;		// Mode démo
	unsigned demo_phase				:3;		// Mode démonstration, phase
	unsigned mode					:2; 	// Code du mode, 0 auto, 1 manul, 2 param
	unsigned erreur					:3;		// Code d'erreur
} cycle;	// Structure de cycle

#endif