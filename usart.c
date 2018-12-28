////////////////////////////////////////////////////////////////
////														////
////			P P E  S - S I    2011-2012					////
////				Projet S I G M A						////
////		   Lycée Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////						usart.c							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de routines pour les			////
////				  communications séries					////
////				  ( 1 : Réception ; 2 : Récep/Emiss )	////
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

//// Définitions ////
unsigned char usart_buffer_recep[USART_TAILLE_BUFFER_RECEP]; 	// Buffer de réception du port série
unsigned char usart_buffer_emiss[USART_TAILLE_BUFFER_EMISS]; 	// Buffer d'émission du port série
unsigned char *usart_ptr_buffer_recep_e;						// Poiteur d'écriture du buffer de reception du port série
unsigned char *usart_ptr_buffer_recep_l;						// Poiteur de lecture du buffer de reception du port série
unsigned char *usart_ptr_buffer_emiss_e;						// Poiteur d'écriture du buffer d'émission du port série
unsigned char *usart_ptr_buffer_emiss_l;						// Poiteur de lecture du buffer d'émission du port série
unsigned char usart_data_to_read;

////////////////////////////////////////////////////////////////
////	Fonction usart_init(void)							////
////	Description : Initialise les ports séries			////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void usart_init(void)
{
	//// Pointeurs de buffers ////
	usart_ptr_buffer_recep_e = usart_buffer_recep;
	usart_ptr_buffer_recep_l = usart_buffer_recep;
	usart_ptr_buffer_emiss_e = usart_buffer_emiss;
	usart_ptr_buffer_emiss_l = usart_buffer_emiss;

	//// Pattes de communication ////
	TRISCbits.TRISC7 = 1;	// RX - En entrée
	TRISCbits.TRISC6 = 1;	// TX - En entrée bien que se soit une sortie
	
	//// Baudrate générator ////
	BAUDCON = 0b00000000;			// Baudrate en mode 8 bits, BRG16 = 0
	SPBRG = 25; 	                // Configure la vitesse (BAUD) 9600
	SPBRGH = 0;
	
	//// Modules USART ////
	// UART
	RCSTA = 0b10010000;			// Active l'USART  CREN=1 et SPEN=1
    TXSTA = 0b00100000;			// Config du module USART en émission, BRGH = 0
   	
    //// Interruptions ////
	// Emission
    PIR1bits.TXIF = 0;				// Mise à zéro des drapeaux
	IPR1bits.TXIP = 0;				// Selectionne basse priorité pour TX (vecteur en 0x18), inutile
	PIE1bits.TXIE = 0;				// IT en emission désactivée, sera activé par la fonction "usart_envoyer_car()"
	// Réception
	PIR1bits.RCIF = 0;				// Drapeaux
	IPR1bits.RCIP = 0;				// Sélectionne basse priorité pour RX (vecteur en 0x18)
	PIE1bits.RCIE = 1;				// IT en réception activées
	
	// RAZ des drapeau
	usart_data_to_read = 0;
}

////////////////////////////////////////////////////////////////////////////////////
////	Fonction usart_etat_buffer_recep()										////
////	Description : Indique si un carractère est disponible dans un buffer	////
////	Paramètres : aucun														////
////	Retour : 0, aucun carractère disponible									////
////			 1ou+, un ou plusieurs carractère sont disponibles				////
////	Auteur : TL																////
////////////////////////////////////////////////////////////////////////////////////
unsigned char usart_etat_buffer_recep()
{
	return (usart_ptr_buffer_recep_e - usart_ptr_buffer_recep_l); // Pointeur d'ecriture moins pointeur lecture
}

////////////////////////////////////////////////////////////////////////////
////	Fonction usart_recevoir_car()									////
////	Description : Renvoi le carractère le plus ancien disponible	////
////				  dans un buffer									////
////	Paramètres : aucun												////
////	Retour : le carractère le plus ancien du buffer selectionné		////
////			 ou 0 si aucun caractère n'est disponible				////
////	Auteur : TL														////
////////////////////////////////////////////////////////////////////////////
unsigned char usart_recevoir_car()
{
	unsigned char usart_car_retour;	// Carractère à renvoyer
	usart_car_retour = *usart_ptr_buffer_recep_l++;			
	usart_gestion_debordements_recep();
	return (usart_car_retour);								// On retourne le caractère lu.
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	Fonction usart_recevoir_chaine(unsigned char *usart_cible, unsigned char usart_car_fin)									////
////	Description : Réceptionne une chaine dans le buffer sélectionné et la termine par NULL (0)								////
////	Paramètres : *usart_cible, pointeur sur l'emplacement mémoire de réception												////
////				 usart_car_fin, carractère de fin de chaine																	////																	////
////	Retour : un pointeur vers la chaine																						////
////	Auteur : TL																												////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned char *usart_recevoir_chaine(unsigned char *usart_cible, unsigned char usart_car_fin)
{
	unsigned char *usart_ptr_retour = usart_cible;	// Définition du pointeur de retour
	unsigned char usart_caractere_temporaire;		// Octet dans lequel on stocke le caractère temporaire

	while ((usart_caractere_temporaire = usart_recevoir_car()) != usart_car_fin)	// Tant qu'on n'a pas obtenu le caractère de fin
		*usart_cible++ = usart_caractere_temporaire; // On le place à l'emplacement cible puis on incrémente
	*usart_cible = 0;			// Enfin, on termine par le caractère de fin
	return (usart_ptr_retour);	// Et on retourne le pointeur de début de chaine
}

////////////////////////////////////////////////////////////////
////	Fonction usart_envoyer_car(unsigned char usart_car)	////
////	Description : Envoi un caractère sur le port série	////
////				  numéro 2								////
////	Paramètres : usart_caractere, caractère à envoyer	////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void usart_envoyer_car(unsigned char usart_caractere)
{
	*usart_ptr_buffer_emiss_e++ = usart_caractere; // On place le caractère à envoyer dans le buffer de sortie, qu'on incrémente
	usart_gestion_debordements_emiss();	// Puis on corrige les éventuels débordements
	PIE1bits.TXIE = 1;		// Activation des interruptions, si le module est libre, cela aura pour effet de générer une interruption
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	Fonction usart_envoyer_chaine(unsigned char *usart_chaine, unsigned char usart_car_fin)			////
////	Description : Envoie une chaine de caractères ( !!!  PRESENTE EN RAM  !!! )						////
////	Paramètres : *usart_chaine, poiteur vers le premier caractère									////
////				 usart_car_fin, caractère de fin de chaine, non émit sur le port					////
////	Retour : Rien																					////
////	Auteur : TL																						////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void usart_envoyer_chaine(unsigned char *usart_chaine, unsigned char usart_car_fin)
{
	while (*usart_chaine != usart_car_fin) // Tant qu'on n'a pas le caractère de fin de chaine
	{
		usart_envoyer_car(*usart_chaine++);	// On émet le caractère
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	Fonction usart_envoyer_chaine_rom(*usart_chaine)												////
////	Description : Envoie une chaine de caractères ( !!!  PRESENTE EN ROM  !!! )						////
////	Paramètres : *usart_chaine, poiteur vers le premier caractère									////
////				 usart_car_fin, caractère de fin de chaine, non émit sur le port					////
////	Retour : Rien																					////
////	Auteur : TL																						////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void usart_envoyer_chaine_rom(const rom char *usart_chaine)
{
	while(*usart_chaine != 0)				// Tant que le carractère de fin de chaine n'est pas trouvé
	{
		usart_envoyer_car(*usart_chaine++);	// On émet le caractère
	}
}

////////////////////////////////////////////////////////////////////////
////	Fonction usart_gestion_debordements_recep(void)				////
////	Description : Détecte et corrige les débordements du buffer ////
////				  de réception du port série					////
////	Paramètres : Aucun											////
////	Retour : Rien												////
////	Auteur : TL													////
////////////////////////////////////////////////////////////////////////
void usart_gestion_debordements_recep(void)
{
	if (usart_ptr_buffer_recep_l >= &usart_buffer_recep[USART_TAILLE_BUFFER_RECEP])	
		usart_ptr_buffer_recep_l = usart_buffer_recep;
	if (usart_ptr_buffer_recep_e >= &usart_buffer_recep[USART_TAILLE_BUFFER_RECEP])	
		usart_ptr_buffer_recep_e = usart_buffer_recep;
}

////////////////////////////////////////////////////////////////
////	Fonction usart_gestion_debordements_emiss(void)		////
////	Description : Détecte et corrige les débordements	////
////				  du buffer d'émission du port série	////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void usart_gestion_debordements_emiss(void)
{
	if (usart_ptr_buffer_emiss_l >= &usart_buffer_emiss[USART_TAILLE_BUFFER_EMISS])	
		usart_ptr_buffer_emiss_l = usart_buffer_emiss;
	if (usart_ptr_buffer_emiss_e >= &usart_buffer_emiss[USART_TAILLE_BUFFER_EMISS])	
		usart_ptr_buffer_emiss_e = usart_buffer_emiss;
}

////////////////////////////////////////////////////////////////
////	Fonction usart_interrupt(void)						////
////	Description : Gère les interruption dues aux ports	////
////				  séries								////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void usart_interrupt(void)
{
	if(PIR1bits.TXIF && PIE1bits.TXIE)
	{
		if(usart_ptr_buffer_emiss_l != usart_ptr_buffer_emiss_e)		// Si un carractère est présent dans le buffer
		{	TXREG = *usart_ptr_buffer_emiss_l++;						// On envoi le carractère le plus ancien du buffer
			usart_gestion_debordements_emiss();							// Gestion des débordements
		}
		else 
			PIE1bits.TXIE = 0;		// On désactive cette source d'interruption
	}
	if(PIR1bits.RCIF)		// Interruption de réception du port série
	{
		*usart_ptr_buffer_recep_e = RCREG;				// Copie du registre de réception
		usart_ptr_buffer_recep_e++;						// On incrémente le pointeur
		usart_data_to_read++;
		usart_gestion_debordements_recep();				// On vérifie les débordements
	}	
}