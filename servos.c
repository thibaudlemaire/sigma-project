////////////////////////////////////////////////////////////////
////														////
////			P P E  S - S I    2011-2012					////
////				Projet S I G M A						////
////		   Lycée Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////					servos.c							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de gestion et de commande 	////
////				  des servos.							////
////														////
////	Créé le : 08/12/2011								////
////	Modifié le : 08/12/2011								////
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


//// Variables ////
unsigned char servos_duty_tilt;
unsigned char servos_duty_roll;
unsigned char servos_consigne_tilt;
unsigned char servos_consigne_roll;
struct
{
	unsigned drapeau			:1;							// Drapeau signifiant un changement de rapport cyclique
	unsigned inter_speed		:1;							// Interruption demandée pour ce servo
	unsigned vitesse			:6;							// Vitesse nominale, choisie par l'utilisateur
	unsigned compteur			:6;							// Compteur d'interrutpions
}servos_speed_tilt;
struct
{
	unsigned drapeau			:1;							// Drapeau signifiant un changement de rapport cyclique
	unsigned inter_speed		:1;							// Interruption demandée pour ce servo
	unsigned vitesse			:6;							// Vitesse nominale, choisie par l'utilisateur
	unsigned compteur			:6;							// Compteur d'interrutpions
}servos_speed_roll;
unsigned char servos_interrupt_phase;						// Phase du rapport cyclique 
unsigned char servos_PRP0;									// Valeur de PR2 pour la phase 0
unsigned char servos_PRP1;									// Phase 1
unsigned char servos_PRP2;									// Phase 2	
unsigned char servos_PRP3;									// Phase 3

/////////////////////////////////////////////////////////////////
////	Fonction servos_init(void)							 ////
////	Description : Initialise les servomoteurs			 ////
////	Paramètres : rien 									 ////
////	Retour : Rien										 ////
////	Auteur : TL											 ////
/////////////////////////////////////////////////////////////////
void servos_init(void)
{
	// Configuration du timer associé aux PWM
	T2CON = 0b00011111;			// Timer 2, prescaler 16, ON, Postscaler 4
	// Interruptions
	PIE1bits.TMR2IE = 1;
	IPR1bits.TMR2IP = 1; 		// HAute priorité, demande de la précision
	
	// Initialisation de la vitesse, la plus rapide :
	servos_set_speed(0,0);
	
	//Initialisation au point zéro
	servos_mip(25,25);
		
	// Initialisation des phases
	servos_interrupt_phase = 0;	
	
	// Polarisation des ports
	TRISBbits.TRISB3 = 0;		// CCP2 en sortie
	TRISCbits.TRISC2 = 0;		// CCP1 en sortie
}

////////////////////////////////////////////////////////////////////
////	Fonction servos_mip(unsigned char, unsigned char)		////
////	Description : Met en place une consigne				 	////
////	Paramètres : pan et tilt entre 0 et 50				 	////
////	Retour : Rien										 	////
////	Auteur : TL											 	////
////////////////////////////////////////////////////////////////////
void servos_mip(unsigned char tilt, unsigned char roll)
{
	servos_mip_tilt(tilt);		// MIP de la consigne
	servos_mip_roll(roll);		// MIP de la consigne
}

/////////////////////////////////////////////////////////////////
////	Fonction servos_mip_tilt(unsigned char)		 		 ////
////	Description : Met en place un rapport cyclique		 ////
////	Paramètres : tilt									 ////
////	Retour : Rien										 ////
////	Auteur : TL											 ////
/////////////////////////////////////////////////////////////////
void servos_mip_tilt(unsigned char tilt)
{
	servos_consigne_tilt = tilt + 61;			// MIP de la consigne
	servos_speed_tilt.inter_speed = 1;		// Activation des changements de vitesse
}

/////////////////////////////////////////////////////////////////
////	Fonction servos_mip_roll(unsigned char)				 ////
////	Description : Met en place un rapport cyclique		 ////
////	Paramètres : roll									 ////
////	Retour : Rien										 ////
////	Auteur : TL											 ////
/////////////////////////////////////////////////////////////////
void servos_mip_roll(unsigned char roll)
{
	servos_consigne_roll = roll + 61;		// MIP de la consigne
	servos_speed_roll.inter_speed = 1;		// Activation des changements de vitesse
}

////////////////////////////////////////////////////////////////////////////////
////	Fonction servos_set_speed(unsigned char)					 		////
////	Description : Configure la vitesse de rotation de la plaque  		////
////	Paramètres : speed, la vitesse de la plaque					 		////
////				0:200°/s ; 1:167°/s ; 2:142°/s ; 3:125°/s -> 35 : 25°/s	////
////	Retour : Rien														////
////	Auteur : TL															////
////////////////////////////////////////////////////////////////////////////////
void servos_set_speed(unsigned char speed_tilt, unsigned char speed_roll)
{
	servos_set_speed_tilt(speed_tilt);
	servos_set_speed_roll(speed_roll);
}


/////////////////////////////////////////////////////////////////////////////////
////	Fonction servos_set_speed_tilt(unsigned char)						 ////
////	Description : Configure la vitesse tilt de rotation de la plaque 	 ////
////	Paramètres : speed, la vitesse de la plaque							 ////
////	Retour : Rien														 ////
////	Auteur : TL															 ////
/////////////////////////////////////////////////////////////////////////////////
void servos_set_speed_tilt(unsigned char speed)
{
	servos_speed_tilt.vitesse = speed;
}

/////////////////////////////////////////////////////////////////////////////////
////	Fonction servos_set_speed_roll(unsigned char)						 ////
////	Description : Configure la vitesse roll de rotation de la plaque 	 ////
////	Paramètres : speed, la vitesse de la plaque							 ////
////	Retour : Rien														 ////
////	Auteur : TL															 ////
/////////////////////////////////////////////////////////////////////////////////
void servos_set_speed_roll(unsigned char speed)
{
	servos_speed_roll.vitesse = speed;
}

/////////////////////////////////////////////////////////////////////////
////	Fonction servos_taches(void)								 ////
////	Description : Gere les taches des servomoteurs				 ////
////	Paramètres : aucun											 ////
////	Retour : Rien												 ////
////	Auteur : TL													 ////
/////////////////////////////////////////////////////////////////////////
void servos_taches(void)
{
	if(servos_speed_tilt.drapeau)		// Si des taches sont à effectuer
	{
		if(servos_duty_tilt > servos_consigne_tilt)
			servos_duty_tilt--;
		else if(servos_duty_tilt < servos_consigne_tilt)
			servos_duty_tilt++;
		else 
		{
			servos_speed_tilt.inter_speed = 0;	// On désactive les changements de vitesse
			servos_speed_tilt.compteur = 0;		// RAZ du compteur en vue des prochaines interruptions
		}
		servos_speed_tilt.drapeau = 0;			// Reset du drapeau
		servos_calcul_pr();						// Calcul des valeurs de PR2
		fonc_affichage_tilt();
	}
	if(servos_speed_roll.drapeau)		// Si des taches sont à effectuer
	{
		if(servos_duty_roll > servos_consigne_roll)
			servos_duty_roll--;
		else if(servos_duty_roll < servos_consigne_roll)
			servos_duty_roll++;
		else 
		{
			servos_speed_roll.inter_speed = 0;	// On désactive les changements de vitesse
			servos_speed_roll.compteur = 0;		// RAZ du compteur en vue des prochaines interruptions
		}		
		servos_speed_roll.drapeau = 0;			// Reset du drapeau
		servos_calcul_pr();						// Calcul des valeurs de PR2
		fonc_affichage_roll();
	}
}

/////////////////////////////////////////////////////////////////////////
////	Fonction servos_calcul_pr(void)								 ////
////	Description : Calcule les valeurs de PR2 durant P0, P1 et P2 ////
////	Paramètres : aucun											 ////
////	Retour : Rien												 ////
////	Auteur : TL													 ////
/////////////////////////////////////////////////////////////////////////
void servos_calcul_pr(void)
{
	servos_PRP0 = servos_duty_roll;							// Premier à être mis à zéro : roll
	servos_PRP1 = 235 - servos_duty_roll;					// Deusième phase
	servos_PRP2 = servos_duty_tilt;							// 3eme
	servos_PRP3 = 235 - servos_duty_tilt;					// 4eme
}
	
/////////////////////////////////////////////////////////////////////////////
////	Fonction servos_interrupt_speed(void)							 ////
////	Description : Gere les interruptions de vitesse des servomoteurs ////
////	Paramètres : aucun											 	 ////
////	Retour : Rien												 	 ////
////	Auteur : TL													 	 ////
/////////////////////////////////////////////////////////////////////////////
void servos_interrupt_speed(void)
{
	if(servos_speed_tilt.inter_speed)
	{
		servos_speed_tilt.compteur++;		// On incrémente le compteur
		if(servos_speed_tilt.compteur >= servos_speed_tilt.vitesse)
		{
			servos_speed_tilt.compteur = 0;		// RAZ du compteur
			servos_speed_tilt.drapeau = 1; 		// Levée du drapeau
		}
	}
	if(servos_speed_roll.inter_speed)
	{
		servos_speed_roll.compteur++;		// On incrémente le compteur
		if(servos_speed_roll.compteur >= servos_speed_roll.vitesse)
		{
			servos_speed_roll.compteur = 0;		// RAZ du compteur
			servos_speed_roll.drapeau = 1; 		// Levée du drapeau
		}
	}
}

/////////////////////////////////////////////////////////////////////////
////	Fonction servos_interrupt(void)								 ////
////	Description : Gere les interruptions des servomoteurs		 ////
////	Paramètres : aucun											 ////
////	Retour : Rien												 ////
////	Auteur : TL													 ////
/////////////////////////////////////////////////////////////////////////
void servos_interrupt(void)
{
	switch (servos_interrupt_phase)
	{
		case 0:
			PORTCbits.RC2 = 0;
			servos_interrupt_phase = 1;
			PR2 = servos_PRP1;
		break;
		case 1:
			PORTBbits.RB3 = 1;
			PR2 = servos_PRP2;
			servos_interrupt_phase = 2;			
		break;
		case 2:
			PORTBbits.RB3 = 0;
			PR2 = servos_PRP3;
			servos_interrupt_phase = 3;	
		break;
		case 3:
			PR2 = 235;
			servos_interrupt_phase = 4;	
		break;
		case 4:
			PR2 = 235;
			servos_interrupt_phase = 5;	
		break;
		case 5:
			PORTCbits.RC2 = 1;
			PR2 = servos_PRP0;
			servos_interrupt_phase = 0;
		break;
	}
}