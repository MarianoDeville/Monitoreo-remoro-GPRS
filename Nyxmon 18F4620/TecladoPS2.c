/********************************************************************************/
/*						DRIVER PARA MANEJO DE TECLADO PS2						*/
/*				Revisión:				1.00									*/
/*				PIC:					PIC18F4620								*/
/*				Teclado:				PS2										*/
/*				Compilador:				MPLAB IDE 8.53 - HI-TECH PICC18 9.50	*/
/*				Fecha de creación:		29/06/2010								*/
/*				Fecha de modificación:	29/09/2010								*/
/*				Autor:					Mariano Ariel Deville					*/
/********************************************************************************/
/*	Es necesario tener cargado el archivo interrupciones.c						*/
/********************************************************************************/
/*						PROTOTIPOS DE FUNCIONES									*/
/*..............................................................................*/
#include	"Teclado.h"				// Tabla para interpretar teclado PS2.		*/
void Interpretar_Teclado(void);		//											*/
void Reseteo_Teclado(void);			//											*/
/********************************************************************************/
/*				*/
/*..............................................................................*/
void Interpretar_Teclado(void)
{											// entonces hago la conversión.
	pos_ps2=11;								// Necesario para la correcta captura de teclas.
	if(lect_ps2==0xf0)						// Solte la tecla que presionaba.
	{
		suelto_tecla_ps2=1;					// Levanto la bandera.
		return;								// Salgo a la interrupción.
	}
	if(suelto_tecla_ps2)					// Omito el caracter de la tecla que se solto.
	{
		suelto_tecla_ps2=0;					// Bajo la bandera.
		if(lect_ps2==0x12||lect_ps2==0x59)
			shift_ps2=0;					// Bajo la bandera del SHIFT.
		if(lect_ps2==0x5a)					// Si es un final de linea proceso el comando.
			INT0IE=0;						// Deshabilito la interrupción.
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x66||(lect_ps2==0x71&&ctrl_ps2))	// Se presionó DEL O DELETE???
	{
		if(pos_str_ps2)						// Si 'pos' es distinto de cero
			pos_str_ps2--;					// vuelvo un lugar.
		rs_str[pos_str_ps2]=0;				// Borro el contenido del lugar.
		ctrl_ps2=0;							// Bajo la bandera de control.
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x74&&ctrl_ps2)			// Se presionó ->???
	{
		ctrl_ps2=0;							// Bajo la bandera de control.
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x6b&&ctrl_ps2)			// Se presionó <-???
	{
		ctrl_ps2=0;							// Bajo la bandera de control.
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x75&&ctrl_ps2)			// Se presionó 'up'???
	{
		ctrl_ps2=0;							// Bajo la bandera de control.
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x72&&ctrl_ps2)			// Se presionó 'down'???
	{
		ctrl_ps2=0;							// Bajo la bandera de control.
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x6c&&ctrl_ps2)			// Se presionó 'home'???
	{
		pos_str_ps2=0;						// Vuelvo al comienzo de la linea.
		ctrl_ps2=0;							// Bajo la bandera de control.
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x7d&&ctrl_ps2)			// Se presionó 'pg up'???
	{
		ctrl_ps2=0;							// Bajo la bandera de control.
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x7a&&ctrl_ps2)			// Se presionó 'pg dn'???
	{
		ctrl_ps2=0;							// Bajo la bandera de control.
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x69&&ctrl_ps2)			// Se presionó 'end'???
	{
		ctrl_ps2=0;							// Bajo la bandera de control.
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x70&&ctrl_ps2)			// Se presionó 'ins'???
	{
		ctrl_ps2=0;							// Bajo la bandera de control.
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0xe0)						// Presionaron una tecla de control.
	{
		ctrl_ps2=1;							// Levanto la bandera de control.
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x58)						// Presionaron la tecla mayusculas.
	{
		mayus_ps2=!mayus_ps2;				// Cambio el balor de la bandera MAYUSCULAS.
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x05)						// Presionaron F1.
	{
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x06)						// Presionaron F2.
	{
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x04)						// Presionaron F3.
	{
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x0c)						// Presionaron F4.
	{
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x03)						// Presionaron F5.
	{
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x0b)						// Presionaron F6.
	{
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x83)						// Presionaron F7.
	{
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x0a)						// Presionaron F8.
	{
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x01)						// Presionaron F9.
	{
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x09)						// Presionaron F10.
	{
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x78)						// Presionaron F11.
	{
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x07)						// Presionaron F12.
	{
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x76)						// Presionaron la tecla ESC borro la linea.
	{
		pos_str_ps2=0;						// Apunto al primer lugar de la cadena.
		rs_str[0]=0;						// Marco el final de la cadena.
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x12||lect_ps2==0x59)		// Presionaron SHIFT
	{
		shift_ps2=1;						// Levanto la bandera del SHIFT.
		return;								// Salgo a la interrupción.
	}
	if(lect_ps2==0x5a)						// Si es un final de linea proceso el comando.
	{
		rs_str[pos_str_ps2]=0;				// Marco el final de la cadena.
		return;								// Salgo a la interrupción.
	}
	cont_ps2=0;
	do										// Recorro la tabla en busca del ASCII.
	{
		CLRWDT();
		if(lect_ps2==ps2[cont_ps2][0])		// Comparo el codigo leido con la tabla.
		{
			if(!mayus_ps2&&!shift_ps2)		// Estaba presionada la tecla MAYUSCULA???
			{
				rs_str[pos_str_ps2]=ps2[cont_ps2][1];
				break;
			}
			if(mayus_ps2&&!shift_ps2)		// Estaba presionada la tecla SHIFT???
			{
				rs_str[pos_str_ps2]=ps2[cont_ps2][2];
				break;
			}
			if(!mayus_ps2&&shift_ps2)		// Estaba presionada la tecla MAYUSCULA y SHIFT???
			{
				rs_str[pos_str_ps2]=ps2[cont_ps2][3];
				break;
			}
			if(mayus_ps2&&shift_ps2)		// Estaba presionada la tecla MAYUSCULA y SHIFT???
				rs_str[pos_str_ps2]=ps2[cont_ps2][4];
			break;							// Una vez encontrado salgo del bucle.
		}
	}while(++cont_ps2<64);					// Busco hasta el último caracter de la tabla.
	if(pos_str_ps2<33)						// Limito a 30 caracteres el comando.
		pos_str_ps2++;						// Voy al siguiente lugar en la cadena.
	rs_str[pos_str_ps2]=0;					// Marco el final de la cadena
	return;									// Salgo a la interrupción.
}
/****************************************************************************************/
/*	Reseteo todas las variables relacionadas con el teclado y habilito la interrupción.	*/
/*......................................................................................*/
void Reseteo_Teclado(void)
{
	CLRWDT();
	rs_str[0]=0;		// Reseteo la cadena.
	pos_str_ps2=0;		// Posicion inicial de la cadena de caracteres.
	lect_ps2=0;			// Vacío la variable lectura del teclado PS2.
	pos_ps2=11;			// Necesario para la correcta captura de teclas.
	mayus_ps2=1;		// Comienzo escribiendo con mayusculas.
	shift_ps2=0;		// Reseteo el SHIFT.
	INT0IF=0;			// Bajo la bandera de la interrupcion.
	INT0IE=1;			// Habilito la interrupción del teclado.
	return;
}
