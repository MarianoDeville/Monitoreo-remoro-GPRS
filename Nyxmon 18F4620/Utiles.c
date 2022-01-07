/********************************************************************************/
/*						DRIVER MODULO GPS GARMIN GPS15H							*/
/*				Revisión:				1.00									*/
/*				PIC:					PIC18F4620								*/
/*				Compilador:				MPLAB IDE 8.53 - HI-TECH PICC18 9.50	*/
/*				Fecha de creación:		29/06/2010								*/
/*				Fecha de finalización:	20/09/2010								*/
/*				Autor:					Mariano Ariel Deville					*/
/********************************************************************************/
unsigned char Comparo_Cadenas(const char *cadena_1,const char *cadena_2);
void Espero_Enter(void);
void Armo_Paquete_Digital(unsigned char *destino);
void Armo_Paquete_Analogico(unsigned char *destino);
unsigned char StrToChar(const char *origen,unsigned char largo);
void IntToStr(unsigned int origen,unsigned char *destino);
bit borrar;
/****************************************************************************/
/*			COMPARO DOS CADENAS DE CARACTERES								*/
/*..........................................................................*/
unsigned char Comparo_Cadenas(const char *cadena_1,const char *cadena_2)
{
	CLRWDT();
	while(*cadena_1||*cadena_2)			// Comparo hasta terminar las dos cadenas.
	{
		if(*cadena_1++!=*cadena_2++)	// Comparo byte a byte de la cadena.
			return 0;					// Hay diferencia asi que salgo y aviso.
	}
	return 1;							// Las cadenas son iguales.
}
/****************************************************************************/
/*		ESPERO QUE SE PRESIONE LA TECLA ENTER EN EL TECLADO PS2.			*/
/*..........................................................................*/
#if defined(DISPLAY)					//
void Espero_Enter(void)
{
	Reseteo_Teclado();
	while(INT0IE)						// Espero un enter del teclado
		DelayMs(50);					// Para el refresco.
	borrar=1;							// Bandera para borrar el display y actualizar.
	return;
}
#endif
/****************************************************************************/
/*	AGREGO LOS DATOS DE LOS SENSORES DIGITALES EN EL PAQUETE A ENVIAR		*/
/*..........................................................................*/
void Armo_Paquete_Digital(unsigned char *destino)
{
	unsigned char armado;
	armado=0;
	while(*destino)						// Busco el primer lugar libre.
	{
		if(armado++>=127)
		{
			*destino=0;					// Marco el final de la cadena.
			return;						// Salgo.
		}
		destino++;						// Hasta no encontrar lugar libre no salgo.
	}
	armado=0b00000010;					// Armo la cebecera.
	armado+=AUX_1;
	armado=(armado<<1);
	armado+=AUX_2;
	armado=(armado<<1);
	armado+=AUX_3;
	armado=(armado<<1);
	armado+=AUX_4;
	armado=(armado<<1);
	armado+=AUX_5;
	armado=(armado<<1);
	armado+=AUX_6;
	*destino++=armado;					// Paso a la memmoria el valor armado.
	armado=0b00000010;					// Armo la cebecera.
	armado+=AUX_7;
	armado=(armado<<1);
	armado+=AUX_8;
	armado=(armado<<1);
	armado+=AUX_9;
	armado=(armado<<1);
	armado+=AUX_10;
	armado=(armado<<1);
	armado+=AUX_11;
	armado=(armado<<1);
	armado+=AUX_12;
	*destino++=armado;					// Paso a la memmoria el valor armado.
	*destino=0;							// Me aseguro de poner un final de linea.
	return;
}
/****************************************************************************/
/*	AGREGO LOS DATOS DE LOS SENSORES ANALÓGICOS EN EL PAQUETE A ENVIAR		*/
/*..........................................................................*/
void Armo_Paquete_Analogico(unsigned char *destino)
{
	unsigned char armado[10];
	volatile char i;
	i=0;
	while(*destino)						// Busco el primer lugar libre.
	{
		if(i++>=127)
		{
			*destino=0;					// Marco el final de la cadena.
			return;						// Salgo.
		}
		destino++;						// Hasta no encontrar lugar libre no salgo.
	}
	for(i=0;i<7;i++)					// Armo los campos analógicos.
	{
		valor_adc=Medir_ADC(i,5);		// Obtengo la medición de temperatura.
		IntToStr(valor_adc,armado);		// Convierto en string.
		*destino=(armado[0]-48)<<4;
		*destino=(armado[1]-48)+(*destino);
		*destino=*destino+32;
		destino++;
		*destino=(armado[2]-48)<<4;
		*destino=(armado[3]-48)+(*destino);
		*destino=*destino+32;
		destino++;
	}
	*destino=0;
	return;
}
/****************************************************************************/
/*			CONVIERTO LA LECTURA EN UNA CADENA DE CARACTERES.				*/
/*..........................................................................*/
unsigned char StrToChar(const char *origen,unsigned char largo)
{
	unsigned char calculo;
	calculo=0;
	while(*origen&&*origen>47&&*origen<58&&largo)
	{
		if(largo==3)
			calculo+=(*origen-48)*100;
		if(largo==2)
			calculo+=(*origen-48)*10;
		if(largo==1)
			calculo+=*origen-48;
		largo--;
	}
	return calculo;
}
/****************************************************************************/
/*			CONVIERTO LA LECTURA EN UNA CADENA DE CARACTERES.				*/
/*..........................................................................*/
void IntToStr(unsigned int origen,unsigned char *destino)
{
	volatile int aux1,aux2,aux3,aux4;			// Variables auxiliares para la conversion.
	CLRWDT();
	if(origen>9999)
	{
		*destino++='-';
		*destino++='-';
		*destino++='-';
		*destino++='-';
		*destino=0;
		return;
	}
	aux1=(origen/1000);							// Calculo la unidad de mil.
	aux2=(origen-aux1*1000)/100;				// Calculo la centena.
	aux3=(origen-aux1*1000-aux2*100)/10;		// Calculo la decena.
	aux4=(origen-aux1*1000-aux2*100-aux3*10);	// Calculo la unidad.
	(*destino++)=aux1+48;						// Unidad de mil.
	(*destino++)=aux2+48;						// Centena.
	(*destino++)=aux3+48;						// Decena.
	(*destino++)=aux4+48;						// Unidad.
	(*destino++)=0;								// Final cadena.
	return;
}
