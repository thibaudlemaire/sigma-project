////////////////////////////////////////////////////////////////
////														////
////			P P E  S - S I    2011-2012					////
////				Projet S I G M A						////
////		   Lycée Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////					servos.h							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de déclarations de servos.c	////
////														////
////	Créé le : 08/12/2011								////
////	Modifié le : 09/12/2011								////
////	Support : PIC 18F2525  -  16 MHz					////
////	Par : 	Thibaud LEMAIRE								////
////														////
////////////////////////////////////////////////////////////////

#ifndef servos_h
#define servos_h


//// Variables ////
extern unsigned char servos_duty_tilt;
extern unsigned char servos_duty_roll;
extern unsigned char servos_consigne_tilt;
extern unsigned char servos_consigne_roll;
extern struct
{
	unsigned drapeau			:1;							// Drapeau signifiant un changement de rapport cyclique
	unsigned inter_speed		:1;							// Interruption demandée pour ce servo
	unsigned vitesse			:6;							// Vitesse nominale, choisie par l'utilisateur
	unsigned compteur			:6;							// Compteur d'interrutpions
}servos_speed_tilt;
extern struct
{
	unsigned drapeau			:1;							// Drapeau signifiant un changement de rapport cyclique
	unsigned inter_speed		:1;							// Interruption demandée pour ce servo
	unsigned vitesse			:6;							// Vitesse nominale, choisie par l'utilisateur
	unsigned compteur			:6;							// Compteur d'interrutpions
}servos_speed_roll;
//// Prototypes ////
void servos_init(void);															// Initialiser les module CCP et les servos
void servos_mip(unsigned char tilt, unsigned char roll);					// Mettre en place une consigne de servos
void servos_mip_tilt(unsigned char tilt);									// Calcul du rapport cyclique tilt
void servos_mip_roll(unsigned char roll);									// Calcul du rapport cyclique roll
void servos_set_speed(unsigned char speed_tilt, unsigned char speed_roll);		// Définie une vitesse de rotation de la plaque
void servos_set_speed_tilt(unsigned char speed);								// Vitesse tilt
void servos_set_speed_roll(unsigned char speed);								// Vitesse roll
void servos_taches(void);														// Taches de servomoteurs
void servos_calcul_pr(void);													// Fonction de calcul des valeurs de PR2
void servos_interrupt_speed(void);												// Interruptions de vitesse
void servos_interrupt(void);													// Interrutpions de postionnement

#endif