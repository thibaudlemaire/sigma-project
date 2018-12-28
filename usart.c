////////////////////////////////////////////////////////////////
////														////
////			P P E  S - S I    2011-2012					////
////				Projet S I G M A						////
////		   Lyc�e Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////						usart.c							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de routines pour les			////
////				  communications s�ries					////
////				  ( 1 : R�ception ; 2 : R�cep/Emiss )	////
////														////
////	Cr�� le : 05/12/2011								////
////	Modifi� le : 05/12/2011								////
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

//// D�finitions ////
unsigned char usart_buffer_recep[USART_TAILLE_BUFFER_RECEP]; 	// Buffer de r�ception du port s�rie
unsigned char usart_buffer_emiss[USART_TAILLE_BUFFER_EMISS]; 	// Buffer d'�mission du port s�rie
unsigned char *usart_ptr_buffer_recep_e;						// Poiteur d'�criture du buffer de reception du port s�rie
unsigned char *usart_ptr_buffer_recep_l;						// Poiteur de lecture du buffer de reception du port s�rie
unsigned char *usart_ptr_buffer_emiss_e;						// Poiteur d'�criture du buffer d'�mission du port s�rie
unsigned char *usart_ptr_buffer_emiss_l;						// Poiteur de lecture du buffer d'�mission du port s�rie
unsigned char usart_data_to_read;

////////////////////////////////////////////////////////////////
////	Fonction usart_init(void)							////
////	Description : Initialise les ports s�ries			////
////	Param�tres : Aucun									////
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
	TRISCbits.TRISC7 = 1;	// RX - En entr�e
	TRISCbits.TRISC6 = 1;	// TX - En entr�e bien que se soit une sortie
	
	//// Baudrate g�n�rator ////
	BAUDCON = 0b00000000;			// Baudrate en mode 8 bits, BRG16 = 0
	SPBRG = 25; 	                // Configure la vitesse (BAUD) 9600
	SPBRGH = 0;
	
	//// Modules USART ////
	// UART
	RCSTA = 0b10010000;			// Active l'USART  CREN=1 et SPEN=1
    TXSTA = 0b00100000;			// Config du module USART en �mission, BRGH = 0
   	
    //// Interruptions ////
	// Emission
    PIR1bits.TXIF = 0;				// Mise � z�ro des drapeaux
	IPR1bits.TXIP = 0;				// Selectionne basse priorit� pour TX (vecteur en 0x18), inutile
	PIE1bits.TXIE = 0;				// IT en emission d�sactiv�e, sera activ� par la fonction "usart_envoyer_car()"
	// R�ception
	PIR1bits.RCIF = 0;				// Drapeaux
	IPR1bits.RCIP = 0;				// S�lectionne basse priorit� pour RX (vecteur en 0x18)
	PIE1bits.RCIE = 1;				// IT en r�ception activ�es
	
	// RAZ des drapeau
	usart_data_to_read = 0;
}

////////////////////////////////////////////////////////////////////////////////////
////	Fonction usart_etat_buffer_recep()										////
////	Description : Indique si un carract�re est disponible dans un buffer	////
////	Param�tres : aucun														////
////	Retour : 0, aucun carract�re disponible									////
////			 1ou+, un ou plusieurs carract�re sont disponibles				////
////	Auteur : TL																////
////////////////////////////////////////////////////////////////////////////////////
unsigned char usart_etat_buffer_recep()
{
	return (usart_ptr_buffer_recep_e - usart_ptr_buffer_recep_l); // Pointeur d'ecriture moins pointeur lecture
}

////////////////////////////////////////////////////////////////////////////
////	Fonction usart_recevoir_car()									////
////	Description : Renvoi le carract�re le plus ancien disponible	////
////				  dans un buffer									////
////	Param�tres : aucun												////
////	Retour : le carract�re le plus ancien du buffer selectionn�		////
////			 ou 0 si aucun caract�re n'est disponible				////
////	Auteur : TL														////
////////////////////////////////////////////////////////////////////////////
unsigned char usart_recevoir_car()
{
	unsigned char usart_car_retour;	// Carract�re � renvoyer
	usart_car_retour = *usart_ptr_buffer_recep_l++;			
	usart_gestion_debordements_recep();
	return (usart_car_retour);								// On retourne le caract�re lu.
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	Fonction usart_recevoir_chaine(unsigned char *usart_cible, unsigned char usart_car_fin)									////
////	Description : R�ceptionne une chaine dans le buffer s�lectionn� et la termine par NULL (0)								////
////	Param�tres : *usart_cible, pointeur sur l'emplacement m�moire de r�ception												////
////				 usart_car_fin, carract�re de fin de chaine																	////																	////
////	Retour : un pointeur vers la chaine																						////
////	Auteur : TL																												////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned char *usart_recevoir_chaine(unsigned char *usart_cible, unsigned char usart_car_fin)
{
	unsigned char *usart_ptr_retour = usart_cible;	// D�finition du pointeur de retour
	unsigned char usart_caractere_temporaire;		// Octet dans lequel on stocke le caract�re temporaire

	while ((usart_caractere_temporaire = usart_recevoir_car()) != usart_car_fin)	// Tant qu'on n'a pas obtenu le caract�re de fin
		*usart_cible++ = usart_caractere_temporaire; // On le place � l'emplacement cible puis on incr�mente
	*usart_cible = 0;			// Enfin, on termine par le caract�re de fin
	return (usart_ptr_retour);	// Et on retourne le pointeur de d�but de chaine
}

////////////////////////////////////////////////////////////////
////	Fonction usart_envoyer_car(unsigned char usart_car)	////
////	Description : Envoi un caract�re sur le port s�rie	////
////				  num�ro 2								////
////	Param�tres : usart_caractere, caract�re � envoyer	////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void usart_envoyer_car(unsigned char usart_caractere)
{
	*usart_ptr_buffer_emiss_e++ = usart_caractere; // On place le caract�re � envoyer dans le buffer de sortie, qu'on incr�mente
	usart_gestion_debordements_emiss();	// Puis on corrige les �ventuels d�bordements
	PIE1bits.TXIE = 1;		// Activation des interruptions, si le module est libre, cela aura pour effet de g�n�rer une interruption
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	Fonction usart_envoyer_chaine(unsigned char *usart_chaine, unsigned char usart_car_fin)			////
////	Description : Envoie une chaine de caract�res ( !!!  PRESENTE EN RAM  !!! )						////
////	Param�tres : *usart_chaine, poiteur vers le premier caract�re									////
////				 usart_car_fin, caract�re de fin de chaine, non �mit sur le port					////
////	Retour : Rien																					////
////	Auteur : TL																						////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void usart_envoyer_chaine(unsigned char *usart_chaine, unsigned char usart_car_fin)
{
	while (*usart_chaine != usart_car_fin) // Tant qu'on n'a pas le caract�re de fin de chaine
	{
		usart_envoyer_car(*usart_chaine++);	// On �met le caract�re
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	Fonction usart_envoyer_chaine_rom(*usart_chaine)												////
////	Description : Envoie une chaine de caract�res ( !!!  PRESENTE EN ROM  !!! )						////
////	Param�tres : *usart_chaine, poiteur vers le premier caract�re									////
////				 usart_car_fin, caract�re de fin de chaine, non �mit sur le port					////
////	Retour : Rien																					////
////	Auteur : TL																						////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void usart_envoyer_chaine_rom(const rom char *usart_chaine)
{
	while(*usart_chaine != 0)				// Tant que le carract�re de fin de chaine n'est pas trouv�
	{
		usart_envoyer_car(*usart_chaine++);	// On �met le caract�re
	}
}

////////////////////////////////////////////////////////////////////////
////	Fonction usart_gestion_debordements_recep(void)				////
////	Description : D�tecte et corrige les d�bordements du buffer ////
////				  de r�ception du port s�rie					////
////	Param�tres : Aucun											////
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
////	Description : D�tecte et corrige les d�bordements	////
////				  du buffer d'�mission du port s�rie	////
////	Param�tres : Aucun									////
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
////	Description : G�re les interruption dues aux ports	////
////				  s�ries								////
////	Param�tres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void usart_interrupt(void)
{
	if(PIR1bits.TXIF && PIE1bits.TXIE)
	{
		if(usart_ptr_buffer_emiss_l != usart_ptr_buffer_emiss_e)		// Si un carract�re est pr�sent dans le buffer
		{	TXREG = *usart_ptr_buffer_emiss_l++;						// On envoi le carract�re le plus ancien du buffer
			usart_gestion_debordements_emiss();							// Gestion des d�bordements
		}
		else 
			PIE1bits.TXIE = 0;		// On d�sactive cette source d'interruption
	}
	if(PIR1bits.RCIF)		// Interruption de r�ception du port s�rie
	{
		*usart_ptr_buffer_recep_e = RCREG;				// Copie du registre de r�ception
		usart_ptr_buffer_recep_e++;						// On incr�mente le pointeur
		usart_data_to_read++;
		usart_gestion_debordements_recep();				// On v�rifie les d�bordements
	}	
}