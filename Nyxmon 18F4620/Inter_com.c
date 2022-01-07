/********************************************************************************/
/*						DRIVER MODULO GPS GARMIN GPS15H							*/
/*				Revisión:				1.00									*/
/*				PIC:					PIC18F4620								*/
/*				Compilador:				MPLAB IDE 8.53 - HI-TECH PICC18 9.50	*/
/*				Fecha de creación:		29/06/2010								*/
/*				Fecha de finalización:	29/09/2010								*/
/*				Autor:					Mariano Ariel Deville					*/
/********************************************************************************/
/*						COMANDOS RECIBIDOS POR TCP.								*/
/*..............................................................................*/
void Ejecutar_Comando(const unsigned char *comando);
void Escribo_Puerto(unsigned char valor, unsigned int direccion);
/********************************************************************************/
/*				INTERPRETO LOS COMANDOS RECIBIDOS POR TCP						*/
/*..............................................................................*/
void Ejecutar_Comando(const unsigned char *comando)
{
	unsigned char accion[4],e,a;
	CLRWDT();
	e=0;
	do									// Separo el comando.
	{
		accion[e++]=*comando++;			// Copio el comando recibido.
	}while(*comando!=':'&&e<4);			// El comando solo debe tener 3 bytes.
	accion[3]=0;						// Marco final de cadena.
	comando++;							// Los dos puntos no los veo.
	if(accion[0]=='A'&&accion[2]>47&&accion[2]<56)
	{
		a=accion[2]-48;					// Convierto el ASCII en un num.
		a+=13;							// Posición de memoria en la que empiezo a escribir.
		e=StrToChar(comando,3);			// Convierto la cadena en un ASCII.
		if(accion[1]=='I')				// Limite inferior?
			Eeprom_Write(a,e);			// Guardo en memoria el valor.
		if(accion[1]=='S')				// Límite superior?
			Eeprom_Write(a+7,e);		// Guardo en memoria el valor.
	}
	if(Comparo_Cadenas(accion,"LVM"))	// Límite velocidad maxima.
	{
		e=StrToChar(comando,3);			// Convierto la cadena en un ASCII.
		Eeprom_Write(11,e);				// Guardo en memoria el valor.
		return;
	}
#if defined(DISPLAY)					//
	if(Comparo_Cadenas(accion,"TXT"))	// Imprimir texto en el display?
	{
		Linea_Lcd(comando,1);			// Imprimo en display el mensaje recibido.
		return;
	}
#endif
	if(Comparo_Cadenas(accion,"FDM"))	// Cambiar el tiempo entre informes?
	{
		e=StrToChar(comando,2);			// Convierto la cadena en un ASCII.
		if(e<61)
		{
			Eeprom_Write(10,e);			// Guardo en EEPROM el valor recibido.
			fdm=e*600;					// Paso el valor.
		}
		return;
	}
	if(Comparo_Cadenas(accion,"PON"))	// Poner en 1 algun pin?
	{
		e=StrToChar(comando,2);			// Convierto la cadena e un ASCII.
		Escribo_Puerto(0,e);			// Escribo en el puerto del pic que corresponde.
		return;
	}
	if(Comparo_Cadenas(accion,"POF"))	// Poner en 0 algun pin?
	{
		e=StrToChar(comando,2);			// Convi<erto la cadena en un ASCII.
		Escribo_Puerto(0,e);			// Escribo en el puerto del pic que corresponde.
		return;
	}
}
/********************************************************************************/
/*						ESCRIBO 0 - 1 EN LOS PUERTOS							*/
/*..............................................................................*/
void Escribo_Puerto(unsigned char valor, unsigned int direccion)
{
	unsigned int lugares;
	lugares=1;							// Igualo a 0b00000000 00000001
	lugares=(lugares<<direccion);		// Desplazo el uno cuantos lugares hagan falta.
	if(valor)							// Debo escribir un uno?
	{
		AUX_1=AUX_1|direccion;
		AUX_2=AUX_2|direccion>>1;
		AUX_3=AUX_3|direccion>>2;
		AUX_4=AUX_4|direccion>>3; 
		AUX_5=AUX_5|direccion>>4;
		AUX_6=AUX_6|direccion>>5;
		AUX_7=AUX_7|direccion>>6;
		AUX_8=AUX_8|direccion>>7;
		AUX_9=AUX_9|direccion>>8;
		AUX_10=AUX_10|direccion>>9;
		AUX_11=AUX_11|direccion>>10;
		AUX_12=AUX_12|direccion>>11;
		return;
	}
	else
	{
		direccion=!direccion;
		AUX_1=AUX_1&direccion;
		AUX_2=AUX_2&direccion>>1;
		AUX_3=AUX_3&direccion>>2;
		AUX_4=AUX_4&direccion>>3;
		AUX_5=AUX_5&direccion>>4;
		AUX_6=AUX_6&direccion>>5;
		AUX_7=AUX_7&direccion>>6;
		AUX_8=AUX_8&direccion>>7;
		AUX_9=AUX_9&direccion>>8;
		AUX_10=AUX_10&direccion>>9;
		AUX_11=AUX_11&direccion>>10;
		AUX_12=AUX_12&direccion>>11;
	}
	return;
}

