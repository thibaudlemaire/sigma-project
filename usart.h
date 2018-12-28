////////////////////////////////////////////////////////////////
////														////
////			P P E  S - S I    2011-2012					////
////				Projet S I G M A						////
////		   Lyc�e Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////						usart.h							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de d�claration de usart.c		////
////														////
////	Cr�� le : 05/12/2011								////
////	Modifi� le : 05/12/2011								////
////	Support : PIC 18F2525  -  16 MHz					////
////	Par : 	Thibaud LEMAIRE								////
////														////
////////////////////////////////////////////////////////////////


#ifndef usart_h
#define usart_h

//// Defines ////
#define USART_TAILLE_BUFFER_RECEP 200 // Taille maximum des paquets du serveur
#define USART_TAILLE_BUFFER_EMISS 40 // Taille maximum des paquets � �mettre

//// Prototypes ////
void usart_init(void);
unsigned char usart_etat_buffer_recep(void);
unsigned char usart_recevoir_car(void);
unsigned char *usart_recevoir_chaine(unsigned char *usart_cible, unsigned char usart_car_fin);
void usart_envoyer_car(unsigned char usart_caractere);
void usart_envoyer_chaine(unsigned char *usart_chaine, unsigned char usart_car_fin);
void usart_envoyer_chaine_rom(const rom char *usart_chaine);
void usart_gestion_debordements_emiss(void);
void usart_gestion_debordements_recep(void);
void usart_interrupt(void);

//// D�finitions ////
extern unsigned char usart_buffer_recep[USART_TAILLE_BUFFER_RECEP]; 	// Buffer de r�ception du port s�rie
extern unsigned char usart_buffer_emiss[USART_TAILLE_BUFFER_EMISS]; 	// Buffer d'�mission du port s�rie
extern unsigned char *usart_ptr_buffer_recep_e;							// Poiteur d'�criture du buffer de reception du port s�rie
extern unsigned char *usart_ptr_buffer_recep_l;							// Poiteur de lecture du buffer de reception du port s�rie
extern unsigned char *usart_ptr_buffer_emiss_e;							// Poiteur d'�criture du buffer d'�mission du port s�rie
extern unsigned char *usart_ptr_buffer_emiss_l;							// Poiteur de lecture du buffer d'�mission du port s�rie
extern unsigned char usart_data_to_read;

#endif
