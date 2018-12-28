////////////////////////////////////////////////////////////////
////														////
////			P P E  S - S I    2011-2012					////
////				Projet S I G M A						////
////		   Lycée Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////						main.c							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Déroulement principal					////
////														////
////	Créé le : 05/12/2011								////
////	Modifié le : 05/12/2011								////
////	Support : PIC 18F2525  -  16 MHz					////
////	Par : 	Thibaud LEMAIRE								////
////														////
////////////////////////////////////////////////////////////////
//// Bits de configuration :								////
//// Oscillator type : INTIO2								////
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

//// Définitions ////
char consigne_tilt;		// Consigne du tilt
char consigne_roll;		// Consigne du roll
struct
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

////////////////////////////////////////////////////////////////
////	Fonction main(void)									////
////	Description : Fonction principale					////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void main(void)
{
	cycle.erreur = 0;			// Drapeau
	cycle.choix_mode = 0;
	cycle.choix_mode_2 = 0;
	cycle.auto_connexion = 0;
	cycle.auto_connecte = 0;
	cycle.manu_choix_axe = 0;
	cycle.manu_choix_axe_2 = 0;
	cycle.manu_tilt = 0;
	cycle.manu_roll = 0;
	cycle.demo = 0;
	cycle.demo_phase = 0;
	cycle.mode = 0;
	////////////////////////////////////
	////// Phase d'initialisation //////
	////////////////////////////////////
	//// Initialisation du PIC ////
	fonc_init_pic();					// Initialise le PIC
	fonc_init_routines();				// Initialise les routines
	//// Tempo de réveil ////
	fonc_set_tempo(TEMPO_REVEIL);		// 1/2 seconde
	while (!fonc_diver.tempo);			// On attend la fin de la tempo
	fonc_diver.tempo = 0;				// RAZ du drapeau	
	lcd_effacer();						// On efface l'écran
	fonc_set_tempo(TEMPO_INIT);			// Initialisation (LCD)
	while (!fonc_diver.tempo)			// Attente de la fin de la tempo
		fonc_taches();					// Execution des taches	
	fonc_unset_tempo();					// Désactivation des eventuelles tempos
		
	////////////////////////////////
	////// Phase de bienvenue //////
	////////////////////////////////
	// RAZ de la position
	fonc_user_tilt(0);							// Mise à zéro de la plauqe
	fonc_user_roll(0);							// ...
	// Message de bienenue
	lcd_positionner(1,0); 						// Positionnement du message de bienvenue
	lcd_afficher_chaine_rom("PPE  2011-2012");	// Message de bienvenue
	// Noms
	lcd_positionner(0,1);
	lcd_afficher_chaine_rom("Thibaud  Lemaire");
	fonc_set_tempo(TEMPO_NOMS);					// Mise en place de la tempo
	while (!fonc_diver.tempo)					// Attente de la fin de la tempo
		fonc_taches();							// Execution des taches	
	lcd_positionner(0,1);
	lcd_afficher_chaine_rom("Pascal  Matteoli");
	fonc_set_tempo(TEMPO_NOMS);					// Initialisation (LCD)
	while (!fonc_diver.tempo)					// Attente de la fin de la tempo
		fonc_taches();							// Execution des taches	
	lcd_positionner(0,1);
	lcd_afficher_chaine_rom("Adrien Mancassol");
	fonc_set_tempo(TEMPO_NOMS);					// Initialisation (LCD)
	while (!fonc_diver.tempo)					// Attente de la fin de la tempo
		fonc_taches();							// Execution des taches	
	// Bienvenue
	lcd_effacer();								// On efface l'écran
	lcd_positionner(2,0); 						// Positionnement du message de bienvenue
	lcd_afficher_chaine_rom("Projet SIGMA");	// Message de bienvenue
	lcd_positionner(3,1); 						// Positionnement du message de bienvenue
	lcd_afficher_chaine_rom("Bienvenue");		// Message de bienvenue
	fonc_set_tempo(TEMPO_MESSAGE);				// Initialisation (LCD)
	while (!fonc_diver.tempo)					// Attente de la fin de la tempo
		fonc_taches();							// Execution des taches	
	
	///////////////////////////
	//// Boucle principale ////
	///////////////////////////
	while(1)
	{
		fonc_gestion_erreurs();						// Gestion et information des eventuelles erreurs
		////////////////////////////////////
		////// Phase de choix du mode //////
		////////////////////////////////////
		lcd_effacer();								// On efface l'écran
		cycle.mode = 0;								// Mode auto
		cycle.choix_mode = 1;						// Debut de la boucle
		while(cycle.choix_mode)
		{	
			fonc_gestion_erreurs();							// Gestion des eventuelles erreurs
			lcd_positionner(0,0); 								// Positionnement du message
			lcd_afficher_chaine_rom("Choix du mode : ");	// Message de choix de mode
			
			///////////////////////////////////
			////// Automatique (serveur) //////
			///////////////////////////////////
			if (cycle.mode == 0)
			{
				lcd_positionner(0,1); 								// Positionnement du message
				lcd_afficher_chaine_rom("< Automatique  >");		// Mode automatique
				cycle.choix_mode_2 = 1;								// Mise en place de la boucle
				while(cycle.choix_mode_2)
				{
					fonc_taches();
					if(fonc_is_front_echantillons(3))
					{
						cycle.mode = 1;
						cycle.choix_mode_2 = 0;
					}
					if(fonc_is_front_echantillons(2))
					{
						cycle.mode = 2;
						cycle.choix_mode_2 = 0;
					}	
					if(fonc_is_front_echantillons(1))
					{
						////////////////////
						// Mode SERVEUR : //
						////////////////////
						cycle.auto_connexion = 1;
						fonc_gestion_erreurs();						// Gestion des eventuelles erreurs
						lcd_effacer();
						lcd_positionner(0,0); 						// Positionnement du message
						lcd_afficher_chaine_rom("Mode automatique");
						lcd_positionner(4,1); 						// Positionnement du message
						lcd_afficher_chaine_rom("Connecte");
						lcd_positionner(0,0); 						// Positionnement du message
						fonc_set_tempo(TEMPO_MESSAGE);				// Initialisation (LCD)
						while (!fonc_diver.tempo)					// Attente de la fin de la tempo
							fonc_taches();							// Execution des taches	
						lcd_effacer();
						lcd_positionner(2,0); 						// Positionnement du message
						lcd_afficher_chaine_rom("Tilt : ");
						lcd_positionner(2,1); 						// Positionnement du message
						lcd_afficher_chaine_rom("Roll : ");
						lcd_positionner(9,0); 
						printf("%d", consigne_tilt);
						lcd_afficher(0b11011111);			// Affichage du "°"
						lcd_afficher_chaine_rom("   ");
						lcd_positionner(9,1); 
						printf("%d", consigne_roll);
						lcd_afficher(0b11011111);			// Affichage du "°"
						lcd_afficher_chaine_rom("   ");
						cycle.auto_connexion = 0;
						cycle.auto_connecte = 1;
						while(cycle.auto_connecte)
						{
							fonc_taches();
							if(fonc_is_front_echantillons(3))			// +
							{
								cycle.erreur = ERREUR_IMPOSSIBLE;
								fonc_gestion_erreurs();
								lcd_effacer();
								lcd_positionner(2,0); 						// Positionnement du message
								lcd_afficher_chaine_rom("Tilt : ");
								lcd_positionner(2,1); 						// Positionnement du message
								lcd_afficher_chaine_rom("Roll : ");
								lcd_positionner(9,0); 
								printf("%d", consigne_tilt);
								lcd_afficher(0b11011111);			// Affichage du "°"
								lcd_afficher_chaine_rom("   ");
								lcd_positionner(9,1); 
								printf("%d", consigne_roll);
								lcd_afficher(0b11011111);			// Affichage du "°"
								lcd_afficher_chaine_rom("   ");
							}
							if(fonc_is_front_echantillons(2))			// -
							{
								cycle.erreur = ERREUR_IMPOSSIBLE;
								fonc_gestion_erreurs();
								lcd_effacer();
								lcd_positionner(2,0); 						// Positionnement du message
								lcd_afficher_chaine_rom("Tilt : ");
								lcd_positionner(2,1); 						// Positionnement du message
								lcd_afficher_chaine_rom("Roll : ");
								lcd_positionner(9,0); 
								printf("%d", consigne_tilt);
								lcd_afficher(0b11011111);			// Affichage du "°"
								lcd_afficher_chaine_rom("   ");
								lcd_positionner(9,1); 
								printf("%d", consigne_roll);
								lcd_afficher(0b11011111);			// Affichage du "°"
								lcd_afficher_chaine_rom("   ");
							}
							if(fonc_is_front_echantillons(1))			// OK
							{
								cycle.erreur = ERREUR_SAISIE;
								fonc_gestion_erreurs();
								lcd_effacer();
								lcd_positionner(2,0); 						// Positionnement du message
								lcd_afficher_chaine_rom("Tilt : ");
								lcd_positionner(2,1); 						// Positionnement du message
								lcd_afficher_chaine_rom("Roll : ");
								lcd_positionner(9,0); 
								printf("%d", consigne_tilt);
								lcd_afficher(0b11011111);			// Affichage du "°"
								lcd_afficher_chaine_rom("   ");
								lcd_positionner(9,1); 
								printf("%d", consigne_roll);
								lcd_afficher(0b11011111);			// Affichage du "°"
								lcd_afficher_chaine_rom("   ");
							}
							if(fonc_is_front_echantillons(0))			// Retour
								cycle.auto_connecte = 0;
						}						
						cycle.choix_mode_2 = 0;
					}
					if(fonc_is_front_echantillons(0))
					{
						cycle.erreur = ERREUR_UP;
						cycle.choix_mode_2 = 0;
					}					
				}
			}
			
			/////////////////////////
			////// Mode manuel //////
			/////////////////////////
			if (cycle.mode == 1)
			{
				lcd_positionner(0,1); 								// Positionnement du message
				lcd_afficher_chaine_rom("<    Manuel    >");		// Mode automatique
				cycle.choix_mode_2 = 1;								// Mise en place de la boucle
				while(cycle.choix_mode_2)
				{
					fonc_taches();
					if(fonc_is_front_echantillons(3))
					{
						cycle.mode = 2;
						cycle.choix_mode_2 = 0;
					}
					if(fonc_is_front_echantillons(2))
					{
						cycle.mode = 0;
						cycle.choix_mode_2 = 0;
					}	
					if(fonc_is_front_echantillons(1))
					{
						///////////////////
						// Mode MANUEL : //
						///////////////////
						lcd_effacer();								// On efface l'écran
						cycle.manu_choix_axe = 1;
						while(cycle.manu_choix_axe)
						{
							fonc_gestion_erreurs();						// Gestion des eventuelles erreurs
							cycle.manu_choix_axe_2 = 1;					// Activation des boucles
							lcd_positionner(0,0); 						// Positionnement du message
							lcd_afficher_chaine_rom("Choix de l'axe :");	
							// Tilt
							lcd_positionner(0,1); 						// Positionnement du message
							lcd_afficher_chaine_rom("<   Axe Tilt   >");
							while(!fonc_is_front_echantillons(3) && !fonc_is_front_echantillons(2) &&  cycle.manu_choix_axe && cycle.manu_choix_axe_2)		// Tant qu'on appuis ni sur + ni sur -
							{
								fonc_taches();
								if(fonc_is_front_echantillons(1))		// OK
								{
									lcd_effacer();
									lcd_positionner(0,0); 						// Positionnement du message
									lcd_afficher_chaine_rom("Reglage manuel :");
									lcd_positionner(2,1); 						// Positionnement du message
									lcd_afficher_chaine_rom("Tilt :");
									lcd_positionner(9,1); 
									printf("%d", consigne_tilt);
									lcd_afficher(0b11011111);			// Affichage du "°"
									cycle.manu_tilt = 1;
									while(cycle.manu_tilt)
									{
										fonc_taches();
										if(fonc_is_front_echantillons(3))			// +
										{
											fonc_user_tilt(consigne_tilt + 1);
											// Partie détection du maintient //
											fonc_set_accel(3);
											while(fonc_valeurs.R3)
											{
												fonc_taches();
												if(fonc_is_front_echantillons(3))
													fonc_user_tilt(consigne_tilt + 1);
											}
										}	
										if(fonc_is_front_echantillons(2))			// -
										{
											fonc_user_tilt(consigne_tilt - 1);
											// Partie détection du maintient //
											fonc_set_accel(2);
											while(fonc_valeurs.R2)
											{
												fonc_taches();
												if(fonc_is_front_echantillons(2))
													fonc_user_tilt(consigne_tilt - 1);
											}
										}	
										if(fonc_is_front_echantillons(1))			// OK --> Erreur
										{
											cycle.erreur = ERREUR_SAISIE;
											fonc_gestion_erreurs();						// Gestion de l'erreur
											lcd_effacer();
											lcd_positionner(0,0); 						// Positionnement du message
											lcd_afficher_chaine_rom("Reglage manuel :");
											lcd_positionner(2,1); 						// Positionnement du message
											lcd_afficher_chaine_rom("Tilt :");
											lcd_positionner(9,1); 
											printf("%d", consigne_tilt);
											lcd_afficher(0b11011111);			// Affichage du "°"
										}
										if(fonc_is_front_echantillons(0))			// Retour
										{
											cycle.manu_choix_axe_2 = 0;
											cycle.manu_tilt = 0;	
										}
									}
								}
								if(fonc_is_front_echantillons(0))		// Retour
									cycle.manu_choix_axe = 0;
							}
							
							// Roll
							lcd_positionner(0,1); 						// Positionnement du message
							lcd_afficher_chaine_rom("<   Axe Roll   >");
							while(!fonc_is_front_echantillons(3) && !fonc_is_front_echantillons(2) && cycle.manu_choix_axe && cycle.manu_choix_axe_2)		// Tant qu'on appuis ni sur + ni sur -
							{
								fonc_taches();
								if(fonc_is_front_echantillons(1))		// Appuis sur OK
								{
									lcd_effacer();
									lcd_positionner(0,0); 						// Positionnement du message
									lcd_afficher_chaine_rom("Reglage manuel :");
									lcd_positionner(2,1); 						// Positionnement du message
									lcd_afficher_chaine_rom("Roll :");
									lcd_positionner(9,1); 
									printf("%d", consigne_roll);
									lcd_afficher(0b11011111);			// Affichage du "°"
									cycle.manu_roll = 1;
									while(cycle.manu_roll)
									{
										fonc_taches();
										if(fonc_is_front_echantillons(3))			// +
										{
											fonc_user_roll(consigne_roll + 1);
											// Partie détection du maintient //
											fonc_set_accel(3);
											while(fonc_valeurs.R3)
											{
												fonc_taches();
												if(fonc_is_front_echantillons(3))
													fonc_user_roll(consigne_roll + 1);
											}
										}	
										if(fonc_is_front_echantillons(2))			// -
										{
											fonc_user_roll(consigne_roll - 1);
											// Partie détection du maintient //
											fonc_set_accel(2);
											while(fonc_valeurs.R2)
											{
												fonc_taches();
												if(fonc_is_front_echantillons(2))
													fonc_user_roll(consigne_roll - 1);
											}
										}	
										if(fonc_is_front_echantillons(1))			// OK --> Erreur
										{
											cycle.erreur = ERREUR_SAISIE;
											fonc_gestion_erreurs();						// Gestion de l'erreur
											lcd_effacer();
											lcd_positionner(0,0); 						// Positionnement du message
											lcd_afficher_chaine_rom("Reglage manuel :");
											lcd_positionner(2,1); 						// Positionnement du message
											lcd_afficher_chaine_rom("Roll :");
											lcd_positionner(9,1); 
											printf("%d", consigne_roll);
											lcd_afficher(0b11011111);			// Affichage du "°"
										}
										if(fonc_is_front_echantillons(0))			// Retour
										{
											cycle.manu_roll = 0;	
											cycle.manu_choix_axe_2 = 0;
										}
									}
								}
								if(fonc_is_front_echantillons(0))		// Appuis sur retour
									cycle.manu_choix_axe = 0;
							}
						}
						cycle.choix_mode_2 = 0;
					}
					if(fonc_is_front_echantillons(0))
					{
						cycle.erreur = ERREUR_UP;						
						cycle.choix_mode_2 = 0;
					}			
				}
			}
			
			////////////////////////////////
			////// Mode demonstration //////
			////////////////////////////////
			if (cycle.mode == 2)
			{
				lcd_positionner(0,1); 								// Positionnement du message
				lcd_afficher_chaine_rom("<Demonstration >");		// Mode automatique
				cycle.choix_mode_2 = 1;
				while(cycle.choix_mode_2)
				{
					fonc_taches();
					if(fonc_is_front_echantillons(3))
					{
						cycle.mode = 0;
						cycle.choix_mode_2 = 0;
					}
					if(fonc_is_front_echantillons(2))
					{
						cycle.mode = 1;
						cycle.choix_mode_2 = 0;
					}	
					if(fonc_is_front_echantillons(1))
					{
						///////////////////////
						//// Démonstration ////
						///////////////////////
						cycle.demo = 1;
						lcd_effacer();
						lcd_positionner(1,0); 							// Positionnement du message
						lcd_afficher_chaine_rom("Demonstration");		// Mode automatique
						lcd_positionner(2,1); 							// Positionnement du message
						lcd_afficher_chaine_rom("en cours...");		// Mode automatique
						cycle.demo_phase = 1;
						servos_set_speed(40,20);			// 25°/s et 50°/s
						cycle.manu_tilt = 0;
						cycle.manu_roll = 0;
						cycle.auto_connecte = 0;
						while(cycle.demo)
						{
							if(cycle.demo_phase == 1)
							{
								fonc_set_tilt(25);
								fonc_set_roll(25);
								cycle.demo_phase = 2;
								fonc_set_tempo(10);					
								while (!fonc_diver.tempo)			
								{
									fonc_taches();	
									if(fonc_is_front_echantillons(3) || fonc_is_front_echantillons(2) || fonc_is_front_echantillons(1) || fonc_is_front_echantillons(0))
									{
										cycle.demo = 0;
										cycle.demo_phase = 0;
										fonc_unset_tempo();
									}
								}
							}
							if(cycle.demo_phase == 2)
							{
								fonc_set_roll(-25);
								cycle.demo_phase = 3;
								fonc_set_tempo(10);					
								while (!fonc_diver.tempo)			
								{
									fonc_taches();	
									if(fonc_is_front_echantillons(3) || fonc_is_front_echantillons(2) || fonc_is_front_echantillons(1) || fonc_is_front_echantillons(0))
									{
										cycle.demo = 0;
										cycle.demo_phase = 0;
										fonc_unset_tempo();
									}
								}
							}
							if(cycle.demo_phase == 3)
							{
								fonc_set_tilt(-25);
								fonc_set_roll(25);	
								cycle.demo_phase = 4;
								fonc_set_tempo(10);					
								while (!fonc_diver.tempo)			
								{
									fonc_taches();	
									if(fonc_is_front_echantillons(3) || fonc_is_front_echantillons(2) || fonc_is_front_echantillons(1) || fonc_is_front_echantillons(0))
									{
										cycle.demo = 0;
										cycle.demo_phase = 0;
										fonc_unset_tempo();
									}
								}
							}
							if(cycle.demo_phase == 4)
							{
								fonc_set_roll(-25);
								cycle.demo_phase = 1;
								fonc_set_tempo(10);					
								while (!fonc_diver.tempo)			
								{
									fonc_taches();	
									if(fonc_is_front_echantillons(3) || fonc_is_front_echantillons(2) || fonc_is_front_echantillons(1) || fonc_is_front_echantillons(0))
									{
										cycle.demo = 0;
										cycle.demo_phase = 0;
										fonc_unset_tempo();
									}
								}
							}
						}
						servos_set_speed(0,0);
						fonc_set_tilt(0);
						fonc_set_roll(0);
						cycle.choix_mode_2 = 0;
					}
					if(fonc_is_front_echantillons(0))
					{
						cycle.erreur = ERREUR_UP;
						cycle.choix_mode_2 = 0;
					}			
				}
			}
		}
	}
}
