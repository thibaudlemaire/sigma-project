////////////////////////////////////////////////////////////////
////														////
////			P P E  S - S I    2011-2012					////
////				Projet S I G M A						////
////		   Lycée Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////					fonctions.h							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de déclaration de fonctions.c	////
////														////
////	Créé le : 05/12/2011								////
////	Modifié le : 05/12/2011								////
////	Support : PIC 18F2525  -  16 MHz					////
////	Par : 	Thibaud LEMAIRE								////
////														////
////////////////////////////////////////////////////////////////

#ifndef fonctions_h
#define fonctions_h

//// Definitons ////
extern struct
{
	unsigned echantillons_compteur	:5;				// Compteur d'interruptions d'echantillonage
	unsigned tempo					:1;				// Flag de la tempo
	unsigned echantillons_taches 	:1;				// Flag de taches d'échantillonage
}fonc_diver;									// Structure foure tout	
extern unsigned char compteur_tempo;		// Compteur pour la tempo
extern union 
{
	struct
	{
		unsigned R0A		:1;				// Bits séparés... Valueur récente
		unsigned R0B		:1;				// ... Valeur ancienne, etc...
		unsigned R1A		:1;
		unsigned R1B		:1;
		unsigned R2A		:1;
		unsigned R2B		:1;
		unsigned R3A		:1;
		unsigned R3B		:1;
	};
	unsigned char echantillons;				// Octet représentatifs
}fonc_echantillons;							// Echantillons du port C
extern struct
{
	unsigned R0				:1;				// Valeurs actuelles stables...
	unsigned R1				:1;				// ...
	unsigned R2				:1;				// ...
	unsigned R3				:1;				// ...
	unsigned R0X			:1;				// Valeurs antérieurs stables...
	unsigned R1X			:1;				// ...
	unsigned R2X			:1;				// ...		
	unsigned R3X			:1;				// ...
}fonc_valeurs;								// Valeurs stables
extern struct
{
	unsigned accel_port		:2;				// Port en demande d'acceleration
	unsigned accel_phase	:1;				// Phases : 0 phase initiale --> 1s, 1 phase d'acceleration, baisse la valeur antérieure 4 fois par secondes pour générer un front
	unsigned accel_taches	:1;				// Drapeau de taches d'acceleration, peridodique, géré par T3, toutes les 20ms
	unsigned accel_set		:1;				// Drapeau d'acceleration activée
}fonc_echantillons_accel;					// Structure de gestion de l'acceleration de touches
extern unsigned char fonc_accel_compteur;			// Compteur servant à l'acceleration

//// Prototypes ////
void fonc_init_pic(void);										// Initialisation du PIC
void fonc_init_routines(void);									// Intialisation des routines
void fonc_taches(void);											// Distribution des taches 
void fonc_gestion_erreurs(void);								// Gestion des erreurs
void fonc_set_tempo(unsigned char fonc_tempo);					// MIP d'une tempo
void fonc_unset_tempo(void);									// Desactiver la tempo
void fonc_interrupt_tempo(void);								// Interruptions de temporisation
void fonc_T3_init(void);										// Initialisation du Timer 3 --> Echantillonage et vitesse servos
void fonc_T3_interrupt(void);									// Interruptions du timer 3
void fonc_init_echantillons(void);								// Taches d'échantillonage
void fonc_taches_echantillons(void);							// Taches d'échantillonage
void fonc_set_accel(unsigned char port);						// Mise en place d'une acceleration de touche
void fonc_interrupt_echantillons(void);							// Interruptions d'echantillonage
unsigned char fonc_is_front_echantillons(unsigned char port);	// Detecteur de front
void fonc_serveur_mip(unsigned char tilt, unsigned char roll); 	// Mise en place d'une consigne
void fonc_user_tilt(char tilt);									// Mise en place d'une consigne par l'utilisateur (Tilt)
void fonc_user_roll(char roll);									// Mise en place d'une consigne par l'utilisateur (ROll)
void fonc_set_tilt(char tilt);									// Met en place la consigne 
void fonc_set_roll(char roll);									// Met en place la consigne
void fonc_affichage_tilt(void);
void fonc_affichage_roll(void);

#endif