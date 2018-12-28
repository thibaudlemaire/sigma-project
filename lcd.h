////////////////////////////////////////////////////////////////
////														////
////			P P E  S - S I    2011-2012					////
////				Projet S I G M A						////
////		   Lyc�e Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////						lcd.h							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de d�claration de lcd.c		////
////														////
////	Cr�� le : 05/11/2011								////
////	Modifi� le : 05/12/2011								////
////	Support : PIC 18F2525  -  16 MHz					////
////	Par : 	Thibaud LEMAIRE								////
////														////
////////////////////////////////////////////////////////////////

#ifndef lcd_h
#define lcd_h

//// Defines ////
// Ports :
#define LCD_D7 LATAbits.LATA0 									// 4 bits de donn�es
#define LCD_D6 LATAbits.LATA1 									// ...
#define LCD_D5 LATAbits.LATA2 									// ...
#define LCD_D4 LATAbits.LATA3 									// ...
#define LCD_E  LATAbits.LATA4									// Validation
#define LCD_RS LATAbits.LATA5									// Bit de choix entre commande et affichage
#define LCD_TRIS_D7 TRISAbits.TRISA0							// TRIS du PORTB
#define LCD_TRIS_D6 TRISAbits.TRISA1							// TRIS du PORTB
#define LCD_TRIS_D5 TRISAbits.TRISA2							// TRIS du PORTB
#define LCD_TRIS_D4 TRISAbits.TRISA3							// TRIS du PORTB
#define LCD_TRIS_E TRISAbits.TRISA4								// TRIS du PORTB
#define LCD_TRIS_RS TRISAbits.TRISA5							// TRIS du PORTB
// Buffer
#define LCD_TAILLE_BUFFER 200									// Taille du buffer de commandes
// Param�tres
#define LCD_DELAY_600NS Nop()								 	// Attendre 600ns, 1 NOP � 4Mhz = 1�S
#define LCD_PARAMETRES_AFFICHAGE_MSB 0x80						// Affecte 1 � RS
#define LCD_PARAMETRES_AFFICHAGE_LSB 0xA0						// Correspond � une tempo de 60us et RS � 1
#define LCD_PARAMETRES_EFFACEMENT 0x40							// Correspond � une tempo de 2ms
#define LCD_PARAMETRES_FONCTIONS 0x10							// Correspond � une tempo de 40us

//// Prototypes ////
void lcd_init(void);
void lcd_taches(void);
void lcd_positionner(unsigned char lcd_colone, unsigned char lcd_ligne);
void lcd_afficher(unsigned char lcd_carractere);
void lcd_afficher_chaine_ram(char *lcd_chaine);
void lcd_afficher_chaine_rom(const rom char *lcd_chaine);
void lcd_effacer(void);
void lcd_verif_debordement_e(void);
void lcd_verif_debordement_l(void);
void lcd_interrupt(void);

//// D�finitions ///
extern struct 	// Structure du buffer
{
	unsigned commande				: 4; 	// Les 4 bits de commande
	// Les types de tempos :
	unsigned tempo_40us				: 1;	// Utilis� pour les commandes
	unsigned tempo_60us				: 1;	// Utilis� pour l'affichage
	unsigned tempo_2ms				: 1;	// Utilis� pour l'effacement
	unsigned com_ou_car				: 1;	// D�finit si la patte RS doit �tre � 1 ou 0 (commande ou carract�re)
}lcd_buffer[LCD_TAILLE_BUFFER];				// D�claration du buffer
extern unsigned char *lcd_buffer_ptr_l; 		// Pointeur du buffer en lecture
extern unsigned char *lcd_buffer_ptr_e;			// Pointeur du buffer en �criture
extern unsigned char lcd_flag_tempo;			// Drapeau qui indique si la temporisation entre deux commande est termin�e			

#endif
