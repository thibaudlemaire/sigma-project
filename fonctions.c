////////////////////////////////////////////////////////////////
////														////
////			P P E  S - S I    2011-2012					////
////				Projet S I G M A						////
////		   Lycée Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////					fonctions.c							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de fonctions diverses			////
////														////
////	Créé le : 05/12/2011								////
////	Modifié le : 05/12/2011								////
////	Support : PIC 18F2525  -  16 MHz					////
////	Par : 	Thibaud LEMAIRE								////
////														////
////////////////////////////////////////////////////////////////

//// Includes ////
#include <p18F2525.h>
#include <stdio.h>
#include "main.h"
#include "lcd.h"
#include "usart.h"
#include "fonctions.h"
#include "interrupts.h"
#include "serveur.h"
#include "servos.h"

//// Definitons ////
struct
{
	unsigned echantillons_compteur	:5;				// Compteur d'interruptions d'echantillonage de 0 à 20...
	unsigned tempo					:1;				// Flag de la tempo
	unsigned echantillons_taches 	:1;				// Flag de taches d'échantillonage
}fonc_diver;										// Structure foure tout	
unsigned char compteur_tempo;						// Compteur pour la tempo
union 
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
}fonc_echantillons;							// Echantillons
struct
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
struct
{
	unsigned accel_port		:2;				// Port en demande d'acceleration
	unsigned accel_phase	:1;				// Phases : 0 phase initiale --> 1s, 1 phase d'acceleration, baisse la valeur antérieure 4 fois par secondes pour générer un front
	unsigned accel_taches	:1;				// Drapeau de taches d'acceleration, peridodique, géré par T3, toutes les 20ms
	unsigned accel_set		:1;				// Drapeau d'acceleration activée
}fonc_echantillons_accel;					// Structure de gestion de l'acceleration de touches
unsigned char fonc_accel_compteur;			// Compteur servant à l'acceleration

////////////////////////////////////////////////////////////////
////	Fonction fonc_init_pic(void)						////
////	Description : Initialise le microcontroleur			////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void fonc_init_pic(void)
{
	//// Initialisation de l'oscillateur ////
	OSCCON = 0b01100000; 				// Fréquence 4MHz, Primary oscillator !!!
	OSCTUNE = 0b01000000; 				// PLL enabled --> 16MHz
	// Initialisation du timer 1
	T1CON = 0b10110001;					// Timer1 activé, FOSC/4, Prescaler 8
	IPR1bits.TMR1IP = 0;				// Basse priorité
	TMR1L = 0xAF;						// Génération d'une interruption toutes les 100ms
	TMR1H = 0x3C;
	PIE1bits.TMR1IE = 0;				// On désactive les interruptions pour le moment
	
	//Initialisation de STDIO.H
	stdout = _H_USER;					// Sortie principale définir par l'utilisateur (Ecran LCD)
}

////////////////////////////////////////////////////////////////
////	Fonction _user_putc(char c)							////
////	Description : Est appelé lors d'une sortie STDOUT	////
////	Paramètres : Carractère à afficher					////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
int _user_putc(char c)
{
	lcd_afficher(c);
}

////////////////////////////////////////////////////////////////
////	Fonction fonc_init_routines(void)					////
////	Description : Initialise les routines				////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void fonc_init_routines(void)
{
	it_init();				// Initialise les interruptions
	lcd_init();				// Initialise l'afficheur
	usart_init();			// Initialise les ports série
	serveur_init();			// Initialise les communications avec le serveur
	servos_init();			// Initialise les servomoteurs
	fonc_init_echantillons();	// Initialise l'echantillonage
	fonc_T3_init();				// Initialise le timer 3 --> Vitesse des servos et échantillonage
}

////////////////////////////////////////////////////////////////
////	Fonction fonc_taches(void)							////
////	Description : Effectue les taches récurentes		////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void fonc_taches(void)
{
	lcd_taches();					// Taches de l'ecran LCD
	serveur_taches(); 				// Taches de communication avec le serveur
	fonc_taches_echantillons();		// Taches d'échantillonage
	servos_taches();				// Taches de vitesses de servos
}

////////////////////////////////////////////////////////////////
////	Fonction fonc_gestion_erreurs(void)					////
////	Description : Effectue la gestion des erreurs		////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void fonc_gestion_erreurs(void)
{
	if(cycle.erreur)
	{
		lcd_effacer();
		lcd_positionner(4,0); 						// Positionnement du message d'erreur
		lcd_afficher_chaine_rom("Erreur !");		// Message d'erreur
		switch(cycle.erreur)
		{
			case ERREUR_SAISIE:						// Erreur de saisie
				lcd_positionner(0,1);
				lcd_afficher_chaine_rom("Saisie incorrect");
			break;
			case ERREUR_UP:
				lcd_positionner(1,1);
				lcd_afficher_chaine_rom("Rien plus haut");
			break;
			case ERREUR_IMPOSSIBLE:
				lcd_positionner(0,1);
				lcd_afficher_chaine_rom("Action impossib.");
			break;
		}
		fonc_set_tempo(TEMPO_ERREUR);				// Affichage de l'erreur
		while (!fonc_diver.tempo)					// Attente de la fin de la tempo
			fonc_taches();							// Execution des taches	
		cycle.erreur = 0;
	}
}

////////////////////////////////////////////////////////////////
////	Fonction fonc_set_tempo(unsigned char)				////
////	Description : Configure la temporisation			////
////	Paramètres : Durée de la tempo en 1/10 sec			////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void fonc_set_tempo(unsigned char fonc_tempo)
{
	fonc_diver.tempo = 0;							// RAZ du drapeau
	PIE1bits.TMR1IE = 1;							// Activation des interruptions
	compteur_tempo = fonc_tempo;				// Nombre d'interuptions
}

////////////////////////////////////////////////////////////////
////	Fonction fonc_unset_tempo()							////
////	Description : Désactive la tempo					////
////	Paramètres : 										////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void fonc_unset_tempo()
{
	PIE1bits.TMR1IE = 0;							// Désactivation des interruptions
	fonc_diver.tempo = 1;
}

////////////////////////////////////////////////////////////////
////	Fonction fonc_interrupt_tempo(void)					////
////	Description : Gestion des interrutptions de tempo	////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void fonc_interrupt_tempo(void)
{
	TMR1L = 0xAF;					// Génération d'une interruption toutes les 100ms
	TMR1H = 0x3C;
	compteur_tempo--;					// On décrémente le compteur
	if (compteur_tempo == 0)			// Si la tempo est finie
	{
		fonc_diver.tempo = 1;			// On leve le drapeau
		PIE1bits.TMR1IE = 0;			// On désactive les interruptions
	}
}

////////////////////////////////////////////////////////////////
////	Fonction fonc_T3_init(void)							////
////	Description : Initialisation du timer 3				////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void fonc_T3_init(void)
{
	TMR3H = 0xFE;
	TMR3L = 0x0B;				// Une interruption toutes les 1ms... sert à l'echantillonage et aux servos
	T3CON = 0b00110001;			// Timer activé, prescaler 8:1
	PIE2bits.TMR3IE = 1;		// Interrutpion activées
	IPR2bits.TMR3IP = 0;		// Basse priorité
}

////////////////////////////////////////////////////////////////
////	Fonction fonc_T3_interrupt(void)					////
////	Description : Gestion des interrutptions du timer 3	////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void fonc_T3_interrupt(void)
{
	TMR3H = 0xFE;
	TMR3L = 0x0B;							// Une interruption toutes les 1ms... sert à l'echantillonage et aux servos
	fonc_interrupt_echantillons();			// Echantillonage
	servos_interrupt_speed();				// Appel de la fonction d'interrution des servo (gestion de la vitesse, toutes les 5ms)
}

////////////////////////////////////////////////////////////////
////	Fonction fonc_init_echantillons(void)				////
////	Description : Initialisation de l'echantillonage	////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void fonc_init_echantillons(void)
{
	fonc_diver.echantillons_compteur = 0;	// Mise à zéro du compteur d'interruptions
	fonc_echantillons.echantillons = 0;
	fonc_valeurs.R0X = 0;		// Mise à zéro des valeurs
	fonc_valeurs.R1X = 0;
	fonc_valeurs.R2X = 0;
	fonc_valeurs.R3X = 0;
	fonc_valeurs.R0 = 0;
	fonc_valeurs.R1 = 0;
	fonc_valeurs.R2 = 0;
	fonc_valeurs.R3 = 0;
}

////////////////////////////////////////////////////////////////
////	Fonction fonc_taches_echantillons(void)				////
////	Description : Gestion des taches d'echantillonage	////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void fonc_taches_echantillons(void)
{
	// Partie accélération //
	if(fonc_echantillons_accel.accel_taches)				// Si des taches sont en cours
	{
		fonc_accel_compteur++;								// On incrémente le compteur
		if(!fonc_echantillons_accel.accel_phase)			// Phase 1, 1 seconde
		{
			if(fonc_accel_compteur >= 15)					// Si 300ms se sont écoulées
			{
				fonc_echantillons_accel.accel_phase = 1;	// Changement de phase
				fonc_accel_compteur = 0;					// RAZ du compteur
			}
		}
		else												// Sinon phase 2
		{
			if(fonc_accel_compteur >= 5)					// Si 1/8 de seconde s'est ecoulé
			{
				fonc_accel_compteur = 0;					// RAZ du compteur
				switch(fonc_echantillons_accel.accel_port)	// Choix du port
				{
					case 0:
						if(fonc_valeurs.R0)								// Si la touche est toujours enfoncés
							fonc_valeurs.R0X = 0;						// On génére le front
						else											// Sinon
							fonc_echantillons_accel.accel_set = 0;		// On désactive l'acceleration
					break;
					case 1:
						if(fonc_valeurs.R1)								// Si la touche est toujours enfoncés
							fonc_valeurs.R1X = 0;						// On génére le front
						else											// Sinon
							fonc_echantillons_accel.accel_set = 0;		// On désactive l'acceleration
					break;
					case 2:
						if(fonc_valeurs.R2)								// Si la touche est toujours enfoncés
							fonc_valeurs.R2X = 0;						// On génére le front
						else											// Sinon
							fonc_echantillons_accel.accel_set = 0;		// On désactive l'acceleration
					break;
					case 3: 
						if(fonc_valeurs.R3)								// Si la touche est toujours enfoncés
							fonc_valeurs.R3X = 0;						// On génére le front
						else											// Sinon
							fonc_echantillons_accel.accel_set = 0;		// On désactive l'acceleration
					break;
				}
			}
		}
		fonc_echantillons_accel.accel_taches = 0;			// RAZ du drapeau
	}
	// Partie échantillonage //
	if(fonc_diver.echantillons_taches)
	{
		fonc_diver.echantillons_taches = 0;							// RAZ du drapeau
		if(fonc_echantillons.R0A == fonc_echantillons.R0B)			// Si la valeur est stable
		{	
			if(fonc_valeurs.R0 != fonc_echantillons.R0A)			// Si il y a changement de valeur
			{
				fonc_valeurs.R0X = fonc_valeurs.R0;					// Mise en mémoire de la valeur antérieure
				fonc_valeurs.R0 = fonc_echantillons.R0A;			// Mise en mémoire de la valeur stable actuelle
			}
		}
		if(fonc_echantillons.R1A == fonc_echantillons.R1B)			// Si la valeur est stable
		{
			if(fonc_valeurs.R1 != fonc_echantillons.R1A)			// Si il y a changement de valeur
			{
				fonc_valeurs.R1X = fonc_valeurs.R1;					// Mise en mémoire de la valeur antérieure
				fonc_valeurs.R1 = fonc_echantillons.R1A;			// Mise en mémoire de la valeur stable actuelle
			}
		}
		if(fonc_echantillons.R2A == fonc_echantillons.R2B)			// Si la valeur est stable
		{
		if(fonc_valeurs.R2 != fonc_echantillons.R2A)				// Si il y a changement de valeur
			{
				fonc_valeurs.R2X = fonc_valeurs.R2;					// Mise en mémoire de la valeur antérieure
				fonc_valeurs.R2 = fonc_echantillons.R2A;			// Mise en mémoire de la valeur stable actuelle
			}
		}
		if(fonc_echantillons.R3A == fonc_echantillons.R3B)			// Si la valeur est stable
		{
		if(fonc_valeurs.R3 != fonc_echantillons.R3A)				// Si il y a changement de valeur
			{
				fonc_valeurs.R3X = fonc_valeurs.R3;					// Mise en mémoire de la valeur antérieure
				fonc_valeurs.R3 = fonc_echantillons.R3A;			// Mise en mémoire de la valeur stable actuelle
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////
////	Fonction fonc_set_accel(void)								////
////	Description : Met en place l'acceleration					////
////	Paramètres : port, le numérot du port a accelerer			////
////	Retour : Rien												////
////	Auteur : TL													////
////////////////////////////////////////////////////////////////////////
void fonc_set_accel(unsigned char port)
{
	fonc_echantillons_accel.accel_phase = 0;
	fonc_echantillons_accel.accel_set = 1;
	fonc_echantillons_accel.accel_port = port;
	fonc_accel_compteur = 0;
}

////////////////////////////////////////////////////////////////////////
////	Fonction fonc_interrupt_echantillons(void)					////
////	Description : Gestion des interrutptions d'echantillonage	////
////	Paramètres : Aucun											////
////	Retour : Rien												////
////	Auteur : TL													////
////////////////////////////////////////////////////////////////////////
void fonc_interrupt_echantillons(void)
{
	fonc_diver.echantillons_compteur++;
	if(fonc_diver.echantillons_compteur == 20)
	{
		fonc_diver.echantillons_compteur = 0;
		fonc_echantillons.echantillons = fonc_echantillons.echantillons<<1;				// Décalage de tous les bits vers la gauche, voir structure
		fonc_echantillons.R0A = !PORTCbits.RC0;											// Mise en mémoire des valeurs actuelles
		fonc_echantillons.R1A = !PORTCbits.RC1;
		fonc_echantillons.R2A = !PORTCbits.RC4;
		fonc_echantillons.R3A = !PORTCbits.RC5;
		fonc_diver.echantillons_taches = 1;												// Levée du drapeau de traitement (taches)
		if(fonc_echantillons_accel.accel_set)											// Si un accélération est en cours
			fonc_echantillons_accel.accel_taches = 1;									// On execute les taches
	}
}

////////////////////////////////////////////////////////////////////////
////	Fonction fonc_is_front_echantillons(unsigned char port)		////
////	Description : Détecte un front dans les valeurs stables		////
////	Paramètres : Le numérot de port								////
////	Retour : 0 ou 1												////
////	Auteur : TL													////
////////////////////////////////////////////////////////////////////////
unsigned char fonc_is_front_echantillons(unsigned char port)
{
	switch(port)
	{
		case 0 :
			if(fonc_valeurs.R0 == 1 && fonc_valeurs.R0X == 0) 		// Si on observe un front montant
			{
				fonc_valeurs.R0X = 1;									// On empeche une double détection du front
				return 1;												// Retour de la valeur
			}
			return 0;
		break;
		case 1 :
			if(fonc_valeurs.R1 == 1 && fonc_valeurs.R1X == 0) 		// Si on observe un front montant
			{
				fonc_valeurs.R1X = 1;									// On empeche une double détection du front
				return 1;												// Retour de la valeur
			}
			return 0;
		break;
		case 2 :
			if(fonc_valeurs.R2 == 1 && fonc_valeurs.R2X == 0) 		// Si on observe un front montant
			{
				fonc_valeurs.R2X = 1;									// On empeche une double détection du front
				return 1;												// Retour de la valeur
			}
			return 0;
		break;
		case 3 :
			if(fonc_valeurs.R3 == 1 && fonc_valeurs.R3X == 0) 		// Si on observe un front montant
			{
				fonc_valeurs.R3X = 1;									// On empeche une double détection du front
				return 1;												// Retour de la valeur
			}
			return 0;
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
////	Fonction fonc_set_mip(uchar, uchar)												////
////	Description : Mise en place d'une nouvelle consigne	en provenance du serveur 	////
////	Paramètres : Tilt et Roll entre 0 et 50											////
////	Retour : Rien																	////
////	Auteur : TL																		////
////////////////////////////////////////////////////////////////////////////////////////////
void fonc_serveur_mip(unsigned char tilt, unsigned char roll) 
{
	if(!cycle.erreur)
	{
		fonc_set_tilt(tilt - 25);
		fonc_set_roll(roll - 25);
	}
}

void fonc_user_tilt(char tilt)
{
	fonc_set_tilt(tilt);
	serveur_envoi_etat();
}
 
void fonc_user_roll(char roll)
{
	fonc_set_roll(roll);
	serveur_envoi_etat();
}

void fonc_set_tilt(char tilt)
{
	if(tilt != consigne_tilt && tilt <= 25 && tilt >= -25)
	{
		consigne_tilt = tilt;
		servos_mip_tilt(tilt + 25);
		// Affichage
		if(cycle.manu_tilt)
		{
			lcd_positionner(9,1); 
			printf("%d", consigne_tilt);
			lcd_afficher(0b11011111);			// Affichage du "°"
			lcd_afficher_chaine_rom("   ");
		}
	}
}

void fonc_set_roll(char roll)
{
	if(roll != consigne_roll && roll <= 25 && roll >= -25)
	{
		consigne_roll = roll;
		servos_mip_roll(-roll + 25);
		// Affichage
		if(cycle.manu_roll)
		{
			lcd_positionner(9,1);  
			printf("%d", consigne_roll);
			lcd_afficher(0b11011111);			// Affichage du "°"
			lcd_afficher_chaine_rom("   ");
		}
	}
}

void fonc_affichage_tilt(void)
{
	if(cycle.auto_connecte)
	{
		char tilt;
		tilt = servos_duty_tilt - 86;
		lcd_positionner(9,0);  
		printf("%d", tilt);
		lcd_afficher(0b11011111);			// Affichage du "°"
		lcd_afficher_chaine_rom("     ");
	}
}

void fonc_affichage_roll(void)
{
	if(cycle.auto_connecte)
	{
		char roll;
		roll = -servos_duty_roll + 86;
		lcd_positionner(9,1);  
		printf("%d", roll);
		lcd_afficher(0b11011111);			// Affichage du "°"
		lcd_afficher_chaine_rom("     ");
	}
}