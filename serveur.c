////////////////////////////////////////////////////////////////
////														////
////			P P E  S - S I    2011-2012					////
////				Projet S I G M A						////
////		   Lycée Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////					serveur.c							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de gestion des communications ////
////				  avec le serveur						////
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

////////////////////////////////////////////////////////////////
////	Fonction serveur_init(void)							////
////	Description : Initialise les communications avec le	////
////				  serveur.								////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void serveur_init(void)
{

}

////////////////////////////////////////////////////////////////
////	Fonction serveur_envoi_etat(void)					////
////	Description : Envoi les informations de position de	////
////				  la plaque au serveur					////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void serveur_envoi_etat(void)
{
	usart_envoyer_car(125);						// Envoi de }
	usart_envoyer_car(consigne_tilt + 25);		// Envoi du Pan au serveur
	usart_envoyer_car(consigne_roll + 25); 		// Envoi du Tilt au serveur
	usart_envoyer_car(124);						// Envoi du carractère de fin |
}

/////////////////////////////////////////////////////////////////
////	Fonction serveur_taches(void)						 ////
////	Description : Effectue les communications C/Serveur	 ////
////	Paramètres : Aucun									 ////
////	Retour : Rien										 ////
////	Auteur : TL											 ////
/////////////////////////////////////////////////////////////////
void serveur_taches(void)
{
	unsigned char fonction;
	if (usart_data_to_read >= 4)		// Si un paquet entier est dans le buffer de réception
	{
		if(cycle.auto_connecte || cycle.auto_connexion)
		{
			fonction = usart_recevoir_car();
			if(fonction == 123) // Si la fonction est "{", il s'agit d'une vitesse
			{
				unsigned char v_tilt = usart_recevoir_car();
				unsigned char v_roll = usart_recevoir_car();				
				servos_set_speed(v_tilt, v_roll);	// Mise en place des vitesses
				usart_recevoir_car();	// On supprime le carractère de fin de trame
			}
			else if (fonction == 125) // Si la fonction est "}", valeurs d'angles
			{
				unsigned char local_tilt = usart_recevoir_car();
				unsigned char local_roll = usart_recevoir_car();
				fonc_serveur_mip(local_tilt, local_roll);	// Réception du tilt et du roll et mise en consigne
				usart_recevoir_car(); // On supprime le carractere de fin de trame
			}
		}
		else
		{
			// Vider le buffer
			while (usart_etat_buffer_recep())	// Tant que le buffer est plein
				usart_recevoir_car();			// On le vide
		}
		usart_data_to_read -= 4;
	}
}
