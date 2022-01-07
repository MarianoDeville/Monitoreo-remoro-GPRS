/********************************************************************************/
/*			MANEJO DE INTERRUPCIONES PARA EL PIC18F4620							*/
/*				Revisión:				1.00									*/
/*				PIC:					PIC18F4620								*/
/*				Compilador:				MPLAB IDE 8.53 - HI-TECH PICC18 9.50	*/
/*				Fecha de creación:		29/06/2010								*/
/*				Fecha de modificación:	14/09/2010								*/
/*				Autor:					Mariano Ariel Deville					*/
/********************************************************************************/
void interrupt isr(void)
{
	unsigned char resp;
	if(RCIF && RCIE)								// Interrupcion RS232.
	{
		TMR1IE=0;
		resp=RCREG;									// Vacío el buffer.
		if(RD5)										// Escucho al MODEM.
		{
			if((resp<=31&&pos)||(resp=='>')||pos>=92)	// Condición de salida.
			{
				RCIE=0;								// Deshabilito la interrupción por RS232.
				CREN=0;								// Deshabilito la recepción continua.
				rs_str[pos]=0;						// Marco final de cadena.
				TMR1IE=1;
				return;								// Salgo de la interrupción.
			}
			if(resp>=32)							// Comienzo el llenado.
				rs_str[pos++]=resp;
			TMR1IE=1;
			return;									// Salgo de la interrupción.
		}
		else										// Escucho al GPS o PC.
		{
			if((resp==13&&pos)||pos>=92)			// Condición de salida.
			{
				RCIE=0;								// Deshabilito la interrupción por RS232.
				CREN=0;								// Deshabilito la recepcion continua.
				rs_str[pos]=0;						// Marco final de cadena.
				MODO_PC;
				TMR1IE=1;
				return;								// Salgo de la interrupción.
			}
			if(resp=='$'||pos)						// Comienzo el llenado.
			{
				if(pos==5&&resp!=control)			// Variable que me define que cadena quiero recibir.
					pos=0;							// Comienzo la carga nuevamente.
				else
					rs_str[pos++]=resp;
				TMR1IE=1;
				return;
			}
			TMR1IE=1;
			return;									// Salgo de la interrupción.
		}
	}
	if(TMR1IE && TMR1IF)							// Interrupción por TMR1.
	{
		TMR1H=0b00001011;							// Configuro el tiempo que tarda en generar
		TMR1L=0b11011011;	
		TMR1IF=0;									// Bajo la bandera de la interrupción.
		if(temporizador++>=(TIEMPO-1))					// Tiempo desborde 10 = 1 seg.
		{
			temporizador=0;							// Reseteo el tiempo.
			envio=1;								// Levanto la bandera de envio.
		}
		return;										// Salgo de la interrupción.
	}
#if defined(DISPLAY)			//
	if(INT0IF && INT0IE)							// Interrupción generada por el teclado PS2.
	{
		if(pos_ps2>=3&&pos_ps2<=10)					// Los bit 3 a 10 se consideran datos.
		{											// Paridad, start y stop son ignorados.
			lect_ps2=(lect_ps2>>1);					// Desplazo los bits un lugar
			if(DATO_PS2)							// Dependiendo del dato que leo en el pin del pic
				lect_ps2=(lect_ps2|0x80);			// escribo un 1 en el bit mas significativo.
		}
		pos_ps2--;									// Voy al siguiente bit.
	   	if(!pos_ps2)								// Final de la cadena de bits??
		{
			Interpretar_Teclado();					// Barro la tabla para identificar el ASCII correspondiente.
			lect_ps2=0;								// y vacio la variable lectura.	
		}
		INT0IF=0;									// Bajo la bandera de la interrupción.
		return;
	}
#endif
	return;											// Salgo de la interrupción.
}
