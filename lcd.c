////////////////////////////////////////////////////////////////
////														////
////			P P E  S - S I    2011-2012					////
////				Projet S I G M A						////
////		   Lycée Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////						lcd.c							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Routines d'affichage					////
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

//// Définitions ///
struct 	// Structure du buffer
{
	unsigned commande				: 4; 	// Les 4 bits de commande
	// Les types de tempos :
	unsigned tempo_40us				: 1;	// Utilisé pour les commandes
	unsigned tempo_60us				: 1;	// Utilisé pour l'affichage
	unsigned tempo_2ms				: 1;	// Utilisé pour l'effacement
	unsigned com_ou_car				: 1;	// Définit si la patte RS doit être à 1 ou 0
}lcd_buffer[LCD_TAILLE_BUFFER];				// Déclaration du buffer
unsigned char *lcd_buffer_ptr_l; 			// Pointeur du buffer en lecture
unsigned char *lcd_buffer_ptr_e;			// Pointeur du buffer en écriture
unsigned char lcd_flag_tempo;				// Drapeau qui indique si la temporisation entre deux commande est terminée			

////////////////////////////////////////////////////////////////
////	Fonction lcd_init(void)								////
////	Description : Initialise l'écran LCD et les 		////
////				  routines d'affichage					////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void lcd_init(void)
{
	// Positionnement en sortie
	TRISA = 0b11000000;
	// Mise en place des pointeurs du buffer
	lcd_buffer_ptr_l = (unsigned char*)&lcd_buffer[0]; // le (unsigned char*) est un cast car le buffer lcd_buffer[] est un champs de bits
	lcd_buffer_ptr_e = (unsigned char*)&lcd_buffer[0]; // tandis que les pointeurs sont de type unsigned char
	// Mise à zero des sorties validation et commande
	LCD_E	= 0; 
	LCD_RS	= 0;
	// Configuration du timer
	T0CON	= 0b01000100;	// Config du T0 en Prescaler 1:32, Timer OFF pour le moment
	lcd_flag_tempo = 0;		// Configuration du drapeau
	// Configuration des interruptions
	INTCONbits.TMR0IE	= 1; 	// Activation des interruptions
	INTCON2bits.TMR0IP	= 0; 	// Prio faible
	// Initialisation de l'écran :
	// 8 bits :
	*lcd_buffer_ptr_e++ = 0b00010011;
	*lcd_buffer_ptr_e++ = 0b00010011;
	*lcd_buffer_ptr_e++ = 0b00010011;
	*lcd_buffer_ptr_e++ = 0b00010010; // Mode 8 bits, permet de passer en 4 bits (DL = 0) + tempo 40us
	// 4 bits :
	*lcd_buffer_ptr_e++ = 0b00000010; // MSB, Mode 4 bits, on reste en 4 bits...
	*lcd_buffer_ptr_e++ = 0b00011000; // LSB, ...mais on configure le LCD sur deux lignes et en 5*7 dots + tempo 2ms
	*lcd_buffer_ptr_e++ = 0b00000000; // MSB, Ecran ON, Curseur OFF, Cligno OFF
	*lcd_buffer_ptr_e++ = 0b00011100; // LSB, Ecran ON, Curseur OFF, Cligno OFF + tempo 40us
	*lcd_buffer_ptr_e++ = 0b00000000; // MSB, Effacer ecran
	*lcd_buffer_ptr_e++ = 0b01000001; // LSB, Effacer ecran + tempo 2ms
	*lcd_buffer_ptr_e++ = 0b00000000; // MSB, Mode entrée et incrément du curseur
	*lcd_buffer_ptr_e++ = 0b00010110; // LSB, Mode entrée et incrément du curseur + tempo 40us
	*lcd_buffer_ptr_e++ = 0b00000000; // MSB, Effacer ecran
	*lcd_buffer_ptr_e++ = 0b01000001; // LSB, Effacer ecran + tempo 2ms
}

////////////////////////////////////////////////////////////////
////	Fonction lcd_taches(void)							////
////	Description : Envoi les données (4 bits) en attente	////
////				  dans le buffer						////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void lcd_taches(void)
{
	if (lcd_flag_tempo == 0)
	{
		if ( lcd_buffer_ptr_l != lcd_buffer_ptr_e ) // Si il y a des donnees dans le buffer (le pointeur de lecture et différent du pointeur d'écriture)
		{
			if (*lcd_buffer_ptr_l & 128)		// Si c'est un carractère
				LCD_RS = 1;				// RS =1 -> Carractère
			else
				LCD_RS = 0;

			if (*lcd_buffer_ptr_l & 8)			// Si le bits de commande n°4 est SET
				LCD_D7 = 1;						// D7 = 1
			else
				LCD_D7 = 0;
				
			if (*lcd_buffer_ptr_l & 4)			// etc...
				LCD_D6 = 1;
			else
				LCD_D6 = 0;
				
			if (*lcd_buffer_ptr_l & 2) 
				LCD_D5 = 1;
			else
				LCD_D5 = 0;

			if (*lcd_buffer_ptr_l & 1) 
				LCD_D4 = 1;
			else
				LCD_D4 = 0;

			LCD_E = 1; 			// Validation
			LCD_DELAY_600NS		// Attente pour la prise en compte
			LCD_E = 0;			// On remet à zero
		
			// Mise en place de la tempo			
			if (*lcd_buffer_ptr_l & 16) // 40µs
			{
				TMR0L = 250;			// 5 avant débordement
				lcd_flag_tempo = 1;		// Drapeau de tempo
				T0CONbits.TMR0ON = 1;	// Activation de la tempo
			}
			else if (*lcd_buffer_ptr_l & 32) //60µs
			{
				TMR0L = 247;			// 8 avant débordement
				lcd_flag_tempo = 1;		// Drapeau de tempo
				T0CONbits.TMR0ON = 1;	// Activation de la tempo
			}
			else if (*lcd_buffer_ptr_l & 64) // 2ms
			{
				TMR0L = 5;				// 250 avant débordement
				lcd_flag_tempo = 1;		// Drapeau de tempo
				T0CONbits.TMR0ON = 1;	// Activation de la tempo
			}
			*lcd_buffer_ptr_l = 0; 		// Remise à zéro de cette case du buffer
			lcd_buffer_ptr_l++; 		// Avance d'un pas du pointeur de lecture
			lcd_verif_debordement_l(); 	// Vérification et rectification des débordements
		}
	}
}

////////////////////////////////////////////////////////////////
////	Fonction lcd_positionner(unsigned char lcd_position)////
////	Description : Positione le curseur du LCD			////
////	Paramètres : lcd_colone, [0;15] colone sur le LCD	////
////				 lcd_ligne, [0;1] ligne sur le LCD		////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void lcd_positionner(unsigned char lcd_colone, unsigned char lcd_ligne)
{
	if (lcd_ligne) // Ligne 2
	{
		*lcd_buffer_ptr_e++ = 12;							// Envoi des bits de la ligne, sans tempo (ligne 2 -> 4)
		lcd_verif_debordement_e();							// Vérification et rectification des débordements du buffer
		*lcd_buffer_ptr_e = lcd_colone;						// Envoi de la colone, 4 bits de poids faible
		*lcd_buffer_ptr_e++ |= LCD_PARAMETRES_FONCTIONS;	// Tempo définie sur 40µs
		lcd_verif_debordement_e();							// Vérification et rectification des débordements du buffer
	}
	else // Ligne 1
	{
		*lcd_buffer_ptr_e++ = 8;							// Envoi des bits de la ligne, sans tempo (ligne 1 -> 0)
		lcd_verif_debordement_e();							// Vérification et rectification des débordements du buffer
		*lcd_buffer_ptr_e = lcd_colone;						// Envoi de la colone, 4 bits de poids faible
		*lcd_buffer_ptr_e++ |= LCD_PARAMETRES_FONCTIONS;	// Tempo définie sur 40µs
		lcd_verif_debordement_e();							// Vérification et rectification des débordements du buffer
	}
}

////////////////////////////////////////////////////////////////
////	Fonction lcd_afficher(unsigned char lcd_caractere)	////
////	Description : Affiche un carractère à l'ecran		////
////	Paramètres : lcd_caractere, carractere à afficher	////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void lcd_afficher(unsigned char lcd_caractere)
{
	unsigned char lcd_msb; 							// Définition des variables temporaires
	unsigned char lcd_lsb; 							// qui contiendrons 4 bits de données sérrées à droite
	lcd_msb = lcd_caractere>>4; 					// Affectation des 4 bits de poids fort
	lcd_lsb = lcd_caractere & 0x0F; 				// Affectation des 4 bits de poids faible
	lcd_msb |= LCD_PARAMETRES_AFFICHAGE_MSB;		// Positionne RS à 1 (masque de 0b10000000)
	lcd_lsb |= LCD_PARAMETRES_AFFICHAGE_LSB; 		// Affectation des tempos et de la valeur RS (masque de 0b10100000)
	*lcd_buffer_ptr_e++ = lcd_msb;					// Place le MSB et sa config dans le buffer
	lcd_verif_debordement_e();						// Vérification et rectification des débordements du buffer
	*lcd_buffer_ptr_e++ = lcd_lsb;					// Place le LSB et sa config dans le buffer
	lcd_verif_debordement_e();
}

////////////////////////////////////////////////////////////////
////	Fonction lcd_afficher_chaine_ram(char *)			////
////	Description : Envoi une chaine en RAM à l'afficheur	////
////	Paramètres : *lcd_chaine, pointeur vers la chaine	////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void lcd_afficher_chaine_ram(char *lcd_chaine)
{
	while(*lcd_chaine != 0)					// Tant que le carractère de fin de chaine n'est pas trouvé
	{
		lcd_afficher(*lcd_chaine++);		// On place dans le buffer le carractère
	}
}

////////////////////////////////////////////////////////////////
////	Fonction lcd_afficher_chaine_rom(const rom char *)	////
////	Description : Envoi une chaine en ROM à l'afficheur	////
////	Paramètres : *lcd_chaine, pointeur vers la chaine	////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void lcd_afficher_chaine_rom(const rom char *lcd_chaine)
{
	while(*lcd_chaine != 0)					// Tant que le carractère de fin de chaine n'est pas trouvé
	{
		lcd_afficher(*lcd_chaine++);		// On place dans le buffer le carractère
	}
}

////////////////////////////////////////////////////////////////
////	Fonction lcd_effacer(void)							////
////	Description : Efface l'afficheur LCD				////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void lcd_effacer(void)
{
	*lcd_buffer_ptr_e++ = 0;					// Envoi des bits de poids forts, sans tempo
	lcd_verif_debordement_e();					// Vérification et rectification des débordements du buffer
	*lcd_buffer_ptr_e++ = 0b01000001;			// Envoi des bits de poids faibles (1), avec tempo de 2ms 
	lcd_verif_debordement_e();
}

////////////////////////////////////////////////////////////////
////	Fonction lcd_verif_debordement_e(void)				////
////	Description : Vérifie si des débordements se sont	////
////				  produits sur le pointeur d'écriture	////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void lcd_verif_debordement_e(void)
{
	if (lcd_buffer_ptr_e >= (unsigned char*)&lcd_buffer[LCD_TAILLE_BUFFER])		// Si le pointeur de lecture dépasse du buffer
		lcd_buffer_ptr_e = (unsigned char*)&lcd_buffer[0]; 						// Remise du pointeur au début du buffer
}

////////////////////////////////////////////////////////////////
////	Fonction lcd_verif_debordement_l(void)				////
////	Description : Vérifie si des débordements se sont	////
////				  produits sur le pointeur de lecture	////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void lcd_verif_debordement_l(void)
{
	if (lcd_buffer_ptr_l >= (unsigned char*)&lcd_buffer[LCD_TAILLE_BUFFER])		// Si le pointeur de lecture dépasse du buffer
		lcd_buffer_ptr_l = (unsigned char*)&lcd_buffer[0]; 						// Remise du pointeur au début du buffer
}

////////////////////////////////////////////////////////////////
////	Fonction lcd_interrupt(void)						////
////	Description : Gère les interruptions causées par le	////
////				  timer 2								////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void lcd_interrupt(void)
{
	if(INTCONbits.TMR0IF)
	{
		T0CONbits.TMR0ON = 0;	// On désactive le timer
		lcd_flag_tempo = 0;		// On met le drapeau de temporisation à zéro, on peut renvoyer une commande.
	}
}