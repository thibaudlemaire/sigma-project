////////////////////////////////////////////////////////////////
////														////
////			P P E  S - S I    2011-2012					////
////				Projet S I G M A						////
////		   Lyc�e Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////					interrupts.h						////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de d�clarations de 			////
////				  interrutps.c							////
////														////
////	Cr�� le : 04/12/2011								////
////	Modifi� le : 04/12/2011								////
////	Support : PIC 18F2525  -  16 MHz					////
////	Par : 	Thibaud LEMAIRE								////
////														////
////////////////////////////////////////////////////////////////

#ifndef interrupt_h
#define interrupt_h

//// Prototypes //// 
void it_init(void);
void it_h_prio(void);
void it_b_prio(void);

#endif