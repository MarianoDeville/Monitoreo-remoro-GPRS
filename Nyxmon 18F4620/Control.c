/********************************************************************************/
/*		MODULO DE MEDICION DE TEMPERATURA Y POSICION CON CONEXCION GPRS			*/
/*				Revisión:				1.00									*/
/*				PIC:					PIC18F4620								*/
/*				MODEM:					SIM340CZ								*/
/*				GPS:					GPS15H									*/
/*				Memoria EEPROM:			24LC256									*/
/*				Teclado:				PS2										*/
/*				Dispaly:				C-51505 2 lineas por 20 caracteres.		*/
/*				Puerto RS232:			1 para comunicación externa.			*/
/*				Compilador:				MPLAB IDE 8.53 - HI-TECH PICC18 9.50	*/
/*				Checksum:				0x6ec1	(ABIERTO)						*/
/*				Fecha de creación:		29/06/2010								*/
/*				Fecha de finalización:	29/09/2010								*/
/*				Autor:					Mariano Ariel Deville					*/
/********************************************************************************/
/*					FORMATO DE LA INFORMACION ENVIADA:							*/
/*------------------------------------------------------------------------------*/
/*		Campo	-				Contenido del campo								*/
/*..............................................................................*/
/*		 0		-		6 bytes con la informacion de la fecha y hora en hexa.	*/
/*		 1		-		4 bytes de long en hexa (0000 0000).					*/
/*		 2		-		1 caracter (N o S).										*/
/*		 3		-		5 bytes de long en hexa (00000 0000).					*/
/*		 4		-		1 caracter (E o W).										*/
/*		 5		-		2 bytes con la identificacion del dispositivo en hexe.	*/
/*		 6		-		3 caracteres con el error (000 a 999 o ---).			*/
/*		 7		-		3 caracteres con la velocidad (000 a 999 o ---).		*/
/*		 8		-		3 caracteres con la altura (000 a 999 o ---).			*/
/*		 9		-		3 caracteres con la direccion (000 a 999 o ---).		*/
/*		10		-		1 caracter indicando el estado del equipo.				*/
/*	  11 al 12	-		2 bytes con los sensores digitales en hexa.				*/
/*	  13 al 19	-		Informacion de los sensores analógicos (5 caracteres).	*/
/*------------------------------------------------------------------------------*/
/*		Comando	-				Descripción										*/
/*..............................................................................*/
/*		TXT:	-		Texto a imprimir por display (40 bytes).				*/
/*		FDM:	-		Tiempo entre informes enviados (en minutos, 03. a 60).	*/
/*		PON:	-		Poner en 1 uno de los pines.							*/
/*		POF:	-		Poner en 0 uno de los pines.							*/
/*		LVM:	-		Límite de velocidad para disparar la alarma(en Km/h).	*/
/*		AI0:	-		Límite inferior sensor analógico 0.						*/
/*		AI1:	-		Límite inferior sensor analógico 1.						*/
/*		AI2:	-		Límite inferior sensor analógico 2.						*/
/*		AI3:	-		Límite inferior sensor analógico 3.						*/
/*		AI4:	-		Límite inferior sensor analógico 4.						*/
/*		AI5:	-		Límite inferior sensor analógico 5.						*/
/*		AI6:	-		Límite inferior sensor analógico 6.						*/
/*		AS0:	-		Límite superior sensor analógico 0.						*/
/*		AS1:	-		Límite superior sensor analógico 1.						*/
/*		AS2:	-		Límite superior sensor analógico 2.						*/
/*		AS3:	-		Límite superior sensor analógico 3.						*/
/*		AS4:	-		Límite superior sensor analógico 4.						*/
/*		AS5:	-		Límite superior sensor analógico 5.						*/
/*		AS6:	-		Límite superior sensor analógico 6.						*/
/*------------------------------------------------------------------------------*/
/*				Organización de la memoria EEPROM:								*/
/*	  direción	-			Contenido											*/
/*..............................................................................*/
/*		0 - 9	-		Identificación del producto.							*/
/*		  10	-		Tiempo entre envios de informes (en minutos).			*/
/*		  11	-		Límite de velocidad maxima (0 a 255).					*/
/*		  12	-		Cantidad de sensores analógicos (0 a 7).				*/
/*		13 - 19	-		Límites superiores para los AD (alarma, 0 a 255).		*/
/*		20 - 26	-		Límites inferiores para los AD (alarma, 0 a 255).		*/
/*		00 - 00 -																*/
/********************************************************************************/
/*						MACROS Y CONFIGURACIÓN DE HARDWARE						*/
/*..............................................................................*/
#define		LC256							// Defino el tipo de memoria usado.	*/
#define		DISPLAY							// Utilización de display.			*/
#define		ID0				0				// Identificacion del dispositivo.	*/
#define		ID1				0				// Identificacion del dispositivo.	*/
#define		ID2				0				// Identificacion del dispositivo.	*/
#define		ID3				0				// Identificacion del dispositivo.	*/
#define		PIC_CLK			20000000		// 20Mhz.							*/
#define		TIEMPO			1800			// Tiempo desborde 10 = 1 seg.		*/
#define		ENTRADA			1				//									*/
#define		SALIDA			0				//									*/
#define 	MODO_GPS		RD4=1;RD5=0		// Conmuto entre el GPS y el MODEM.	*/
#define		MODO_MODEM		RD5=1;RD4=0		// Conmuto entre el GPS y el MODEM.	*/
#define		MODO_PC			RD5=0;RD4=0		// Conmuto entre el GPS y el MODEM.	*/
#define		RS232_OFF		RCIE=0;CREN=0	//									*/
#define		RS232_ON		CREN=1;RCIE=1	//									*/
/*------------------------------------------------------------------------------*/
/*				Defino los nombres de los pines de E/S							*/
/*..............................................................................*/
#define		DATO_PS2		RD1				// Datos desde un teclado PS2.		*/
#define		CLOCK_PS2		RB0				// Clock desde un teclado PS2.		*/
#define		DCD				RD6				// Entrada DCD desde el MODEM.		*/
#define		MODEM_RESET		RE2				// Manejo el regulador del MODEM.	*/
#define		POWER_MODEM		RD3				// Manejo el POWER del MODEM.		*/
#define		LED				RD7				// LED DEBUG.						*/
#define		AUX_1			RC5				// Sensor de la puerta del camion.	*/
#define		AUX_2			RC1				//									*/
#define		AUX_3			RD2				//									*/
#define		AUX_4			RC2				//									*/
#define		AUX_5			RD0				//									*/
#define		AUX_6			RA0				//									*/
#define		AUX_7			RA1				//									*/
#define		AUX_8			RA2				//									*/
#define		AUX_9			RA5				//									*/
#define		AUX_10			RE0				//									*/
#define		AUX_11			RE1				//									*/
#define		AUX_12			RE2				//									*/
#define		NET_STATE		RB1				// Estado de conección del MODEM.	*/
#define		GPS_CLOCK		RC0				// Oscilador de 1 Hz del GPS.		*/
/********************************************************************************/
/*						VARIABLES GLOBALES										*/
/*..............................................................................*/
unsigned char rs_str[93],pos;				// Variables para la recepción RS232.
bit consola,bis;							// Declaracion de banderas.
unsigned char cant_sens,envio,control,rst;	// Variables para los sensores de temperatura.
unsigned int pos_mem,temporizador,fdm;		// 
unsigned int valor_adc;						// Almaceno la temperatura con decimales.
/********************************************************************************/
#include	"htc.h"				// Necesario para el compilador.				*/
#include	"delay.c"			// Rutinas de demoras.							*/
#include	"RS232.c"			// Comunicación por puerto serie.				*/
#if defined(DISPLAY)			//												*/
	#include	"lcd.c"			// Driver del display LCD.						*/
	#include	"TecladoPS2.c"	// Rutina de interpretación PS2.				*/
#endif							//												*/
#include	"Interrupciones.c"	// Manejo de interrupciones (TMR,RS232,RB0).	*/
#include	"adc.c"				// Libreria para utilizar el conversor ADC.		*/
#include	"eeprom.c"			// Manejo memoria eeprom.						*/
#include	"I2C.c"				// Necesaria para el manejo de 24LC256.			*/
#include	"Utiles.c"			// Algunas funciones de utilidad.				*/
#include	"Inter_com.c"		// Interpreto los comandos recibidos.			*/
#include	"24LC256.c"			// Manejo memoria eeprom externa.				*/
#include	"SIM300CZ.c"		// Driver del MODEM GSM/GPRS.					*/
#include	"GPS15H.c"			// Obtención de datos del GPS GARMIN 15H.		*/
#if defined(DISPLAY)			//												*/
	#include	"MenuServ.c"	// Menu de mant. utilizo display y teclado.		*/
#endif							//												*/
/********************************************************************************/
/*						PALABRAS DE CONFIGURACIÓN.								*/
/*..............................................................................*/
__CONFIG(1,IESOEN & FCMEN & HS);						//						*/
__CONFIG(2,BOREN & BORV21 & PWRTEN & WDTEN & WDTPS2K);	//						*/
__CONFIG(3,MCLREN & LPT1EN & PBDIGITAL);				//						*/
__CONFIG(4,XINSTDIS & DEBUGDIS & LVPDIS & STVREN);		//						*/
__CONFIG(5,UNPROTECT);									//						*/
//__CONFIG(6,WRTEN & WPALL);							//						*/
//__CONFIG(7,TRPALL);									//						*/
__IDLOC(00000000);										// ID del programa.		*/
/********************************************************************************/
void main(void)
{
	unsigned char str_aux[17],paquete[PAGINA];	// Cadenas auxiliar de uso múltiple.
	unsigned char i,vaciar;						// Variable auxiliar de uso múltiple.
/********************************************************************************/
/*				Configuración de los puertos									*/
/*..............................................................................*/
	TRISA0=ENTRADA;			// Entrada analogica 1.								*/
	TRISA1=ENTRADA;			// Entrada analogica 2.								*/
	TRISA2=ENTRADA;			// Entrada analogica 3.								*/
	TRISA3=ENTRADA;			// Entrada analogica 4.								*/
	TRISA4=SALIDA;			// 													*/
	TRISA5=ENTRADA;			// Entrada analogica 5.								*/
/*..............................................................................*/
	TRISB0=ENTRADA;			// PS2 - CLOCK.										*/
	TRISB1=ENTRADA;			// Estado del MODEM (NET STATE).					*/
	TRISB2=SALIDA;	   		// Salida para el LCD RS.							*/
	TRISB3=SALIDA;   		// Salida para el LCD E.							*/
	TRISB4=SALIDA; 	  		// Salida para el LCD AD4.							*/
	TRISB5=SALIDA;   		// Salida para el LCD AD5.							*/
	TRISB6=SALIDA;   		// Salida para el LCD AD6.							*/
	TRISB7=SALIDA;   		// Salida para el LCD AD7.							*/
/*..............................................................................*/
	TRISC0=ENTRADA;			// Salida de clock del GPS.							*/
	TRISC1=ENTRADA;			// Entrada auxiliar 2.								*/
	TRISC2=ENTRADA;			// Entrada auxiliar 4.								*/
	TRISC3=ENTRADA;			// I2C - SCL serial clock.							*/
	TRISC4=ENTRADA;			// I2C - SDA serial data.							*/
	TRISC5=ENTRADA;			// Entrada auxiliar 1.								*/
	TRISC6=SALIDA;			// RS232 - Salida TX.								*/
	TRISC7=ENTRADA;			// RS232 - Entrada RX.								*/
/*..............................................................................*/
	TRISD0=ENTRADA;			// Entrada auxiliar 5.								*/
	TRISD1=ENTRADA;			// PS2 - DATOS.										*/
	TRISD2=ENTRADA;			// Entrada auxiliar 3.								*/
	TRISD3=SALIDA;			// Manejo POWER del MODEM							*/
	TRISD4=SALIDA;			// Habilitación RS232 para el MODEM.				*/
	TRISD5=SALIDA;			// Habilitación RS232 para el GPS.					*/
	TRISD6=ENTRADA;			// DCD del MODEM.									*/
	TRISD7=SALIDA;			// DEBUG.											*/
/*..............................................................................*/
	TRISE0=ENTRADA;			// Entrada analogica 6.								*/
	TRISE1=ENTRADA;			// Entrada analogica 7.								*/
	TRISE2=SALIDA;			// Reset del MODEM.									*/
/********************************************************************************/
/*			TIMER 0 - NO UTILIZADO												*/
/*..............................................................................*/
	T0CS=0;					// Oscilador interno.								*/
	T0SE=0;					// Flanco ascendente.								*/
	PSA=1;					// Asigno el preescaler a WDT.						*/
	TMR0IF=0;				// Bajo la bandera de la interrupción.				*/
/********************************************************************************/
/*			TIMER 1 - Lo utilizo para calcular los 10 min entre envio y envio.	*/
/*..............................................................................*/
	T1CKPS0=1; 				// Preescaler TMR1 a 1:8.							*/
	T1CKPS1=1; 				//													*/
	T1SYNC=1;				// No sincronizo con clock externo.					*/
	T1OSCEN=1;				// Oscilador habilitado.							*/
	TMR1CS=0;  				// Reloj interno Fosc/4.							*/
	TMR1IF=0;				// Bajo la bandera de la interrupción.				*/
	TMR1ON=1;				// Enciendo el TMR1.								*/
	TMR1H=0b00001011;		// Configuro el tiempo que tarda en generar			*/
	TMR1L=0b11011011;		// la interrupcion (100 mseg).						*/
/********************************************************************************/
/*			TIMER 2 - NO UTILIZADO												*/
/*..............................................................................*/
	TMR2ON=0;				// Timer 2 apagado.									*/
	T2CKPS0=0;				// Configuro el Preescaler.							*/
	T2CKPS1=0;				// 													*/
	TMR2IF=0;				// Bajo la bandera de la interrupción.				*/
/********************************************************************************/
/*			Configuración de las interrupciones									*/
/*..............................................................................*/
	IPEN=0;					// Deshabilito las prioridades para las int.		*/
	GIE=1;					// Utilizo interrupciones.							*/
	PEIE=1;					// Interrupcion externa habilitada.					*/
	INT0IE=0;				// Interrupcion RB0/INT deshabilitada.				*/
	INT1IE=0;				// Interrupcion RB1/INT deshabilitada.				*/
	INT2IE=0;				// Interrupcion RB2/INT deshabilitada.				*/
	TMR0IE=0;				// Interrupcion desborde TMR0 deshabilitada.		*/
	TMR1IE=1;				// Interrupcion desborde TMR1 habilitada.			*/
	TMR2IE=0;				// Interrupcion desborde TMR2 deshabilitada.		*/
	CCP1IE=0;				// CCP1 Interrupt disable.							*/
	CCP2IE=0;				// CCP2 Interrupt disable.							*/
	CMIE=0;					// Comparator Interrupt disable.					*/
	EEIE=0;					// EEPROM Write Operation Interrupt disable.		*/
	SSPIE=0;				// Interrupcion por comunicacion I2C.				*/
	PSPIE=0;				// Slave Port Read/Write Interrupt disable.			*/
	BCLIE=0;				// Bus Collision Interrupt disable.					*/
	ADIE=0;					// Interrupcion del conversor AD deshabilitada.		*/
	RBIE=0;					// Interrupcion por RB deshabilitada.				*/
	RCIE=0;					// Interrupcion recepcion USART deshabilitada.		*/
 	INTEDG0=0;				// Interrupcion en el flanco descendente de RB0.	*/
	RBPU=1;					// RB pull-ups estan deshabilitadas.				*/
/********************************************************************************/
	cant_sens=6;									// Cantidad de sensores analógicos.
	MODEM_RESET=1;
	POWER_MODEM=1;									// MODEM apagado hasta necesitarlo.
	MODO_PC;										// Arranco con comunicación a la PC.
	Serial_Setup(9600);								// Setea el puerto serie.
	I2C_Setup(200);									// Inicializo la comunicación I2C.
	Setup_Eeprom();									// Guardo en memoria EEPROM los valores iniciales.
	LED=1;											// Led encendido.
#if defined(DISPLAY)								//
	Menu_Servicio();								// Presiono ENTER para entrar en el menú de servicio.
	Lcd_Clear();									// Limpio el display.
#endif
	MODEM_RESET=0;
	Leo_Coordenadas(paquete);						// Cargo hora, fecha y coordenadas
	Armo_Paquete_Digital(paquete);					// Agrego la información digital.
	Armo_Paquete_Analogico(paquete);				// Agrego la información analógica.
	paquete[PAGINA-1]=0;							// Me aseguro el final de cadena.
	if(!Envio_Informe(paquete))						// Mando el paquete informando el inicio del programa.
		Guardo_Mem_Aux(paquete);					// Imposible enviar así que guardo en 24LC256.
	temporizador=TIEMPO-50;							// Primera transmisión se hace a los 5 segundos.
	envio=0;										// Inicializo variables.
	while(!envio)									// Espero a que se levante la bandera de transmisión.
		CLRWDT();
	for(;;)
	{
		if(envio)									// Es tiempo de envío?
		{
			envio=0;								// Bajo bandera de envio de datos.
			if(Leo_Coordenadas(paquete))			// Hay coordenadas disponibles?
			{
				paquete[32]='R';					// El paquete salió en Régimen
				Armo_Paquete_Digital(paquete);		// Agrego la información digital.
				Armo_Paquete_Analogico(paquete);	// Agrego la información analógica.
				paquete[PAGINA-1]=0;				// Me aseguro el final de la cadena.
				if(!Envio_Informe(paquete))			// Mando el paquete armado en la ram del PIC.
					Guardo_Mem_Aux(paquete);		// Imposible enviar así que guardo en 24LC256.
			}
		}
		else
		{
			i=Info_Extra('V',str_aux);				// Obtengo la velocidad instantánea.
			if(i>90)								// Supero los 90 Km/h?
				envio=1;
			pos_mem=Recupero_Mem_Aux(paquete);		// Obtengo la primer pagina libre.
			vaciar=1;
			while(pos_mem<5000&&!envio)				// Vacio mientras no se levante la bandera y tenga algo en memoria y señal.
			{
				paquete[32]='M';					// El paquete salió de la memoria.
				paquete[PAGINA-1]=0;				// Me aseguro el final de la cadena.
				vaciar=Envio_Buffer(paquete,vaciar);
				if(vaciar)							// Envio correcto??
					Borrar_24LC256(pos_mem);		// Borro el dato enviado.
				else
					break;
				pos_mem=Recupero_Mem_Aux(paquete);	// Obtengo la primer pagina libre.
			}
			if(envio||vaciar!=1)
				Modem_Power(0,1);					// Apago el MODEM.
		}
	}
}
