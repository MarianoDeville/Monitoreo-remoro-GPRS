/********************************************************************************/
/*						DRIVER MODULO GPS GARMIN GPS15H							*/
/*				Revisión:				1.00									*/
/*				PIC:					PIC18F4620								*/
/*				Compilador:				MPLAB IDE 8.53 - HI-TECH PICC18 9.50	*/
/*				Fecha de creación:		29/06/2010								*/
/*				Fecha de finalización:	29/09/2010								*/
/*				Autor:					Mariano Ariel Deville					*/
/********************************************************************************/
void Comandos_GPS(unsigned char comando);
unsigned char Leo_Coordenadas(unsigned char *armado);
void Coordenadas(unsigned char *armado);
void Altura(unsigned char *valor);
unsigned char Info_Extra(unsigned char info,unsigned char *valor);
void CR_LF(void);
/********************************************************************************/
/*		COMANDOS PARA LA CONFIGURACION DEL FORMATO DE LOS DATOS DEL GPS			*/
/*..............................................................................*/
void Comandos_GPS(unsigned char comando)
{
	const static unsigned char pgrmo[]="$PGRMO,";
	CLRWDT();
	MODO_GPS;						// Habilito el RS232 para el GPS.
	DelayMs(2);
	if(comando=='C')				// Configurar?
	{
		PutStr(pgrmo);
		PutStr("GPGSA,0");
		CR_LF();
		PutStr(pgrmo);
		PutStr("GPGSV,0");
		CR_LF();
		PutStr(pgrmo);
		PutStr("GPRMC,0");
		CR_LF();
		PutStr(pgrmo);
		PutStr("GPVTG,0");
		CR_LF();
		PutStr(pgrmo);
		PutStr("LCGLL,0");
		CR_LF();
		PutStr(pgrmo);
		PutStr("LCVTG,0");
		CR_LF();
		PutStr(pgrmo);
		PutStr("PGRMB,0");
		CR_LF();
		PutStr(pgrmo);
		PutStr("GPGGA,1");			// Información de la altura.
		CR_LF();
		PutStr(pgrmo);
		PutStr("PGRME,1");			// Información del error de posicionamiento.
		CR_LF();
		PutStr(pgrmo);
		PutStr("PGRMF,1");			// Información de la fecha, hora y coordenadas.
		CR_LF();
		PutStr(pgrmo);
		PutStr("PGRMT,0");
		CR_LF();
		PutStr(pgrmo);
		PutStr("PGRMV,0");
		CR_LF();
		PutStr(pgrmo);
		PutStr("PGRMM,0");
		CR_LF();
		PutStr(pgrmo);
		PutStr("GPGLL,0");
		CR_LF();
		PutStr("$PGRMI,3121.3933,S,06412.1642,W,050710,191028");
		CR_LF();
		PutStr("$PGRMC,A,446.0,100,,,,,,A,4,1,2,9,30");
		CR_LF();
		PutStr("$PGRMC1,1,1,2,300.0,200,2,1,W,N,1,1,1,1");
		CR_LF();
		DelayMs(2);
		MODO_PC;
		return;						// Salgo de la funcion.
	}
	if(comando=='A')				// Autolocate?
		PutStr("$PGRMI,,,,,,,A");
	if(comando=='R')				// Reset?
		PutStr("$PGRMI,,,,,,,R");
	CR_LF();
	DelayMs(2);
	MODO_PC;
	return;
}
/********************************************************************************/
/*				OBTENGO LAS COORDENADAS, FHECHA Y HORA							*/
/*..............................................................................*/
unsigned char Leo_Coordenadas(unsigned char *armado)
{
	static const unsigned char comando[]="$PGRMF";
	unsigned char aux[6],paridad;
	volatile unsigned char campo;
	MODO_GPS;							// Habilito el RS232 para el GPS.
	pos=0;
	campo=0;
	control='F';						// Comando que deseo leer.
	DelayMs(2);
	RS232_ON;								// Habilito la interrupcion por RS232.
	while(RCIE)							// Espero que este cargada la lectura del GPS.
	{
		DelayMs(10);
		if(++campo>250)					// tiempo * DelayMs = 2.5 segundos.
		{
			RS232_OFF;
			pos=0;
			for(campo=0;campo<19;campo++)	// Relleno con ceros en todos los campos.
			{
				*armado++=32;
			}
			MODO_PC;
			*armado=0;					// Marco el principio de la cadena con un NULL.
			return 0;					// Excedió el tiempo de respuesta.
		}
	}
	MODO_PC;							// Dejo de eschuchar el GPS.
	for(pos=0;pos<6;pos++)				// Compruebo el comando recibido.
	{
		if(rs_str[pos]!=comando[pos])
		{
			*armado=0;					// Marco el principio de la cadena con un NULL.
			MODO_PC;
			return 0;					// No se puede procesar, el comando no es reconocido.
		}
	}
	pos=0;								// Comienzo a escribir en la cadena desde el primer lugar.
	campo=0;
	paridad=0;
	do									// Almaceno y acomodo el la eeprom los datos recibidos.
	{
		if(rs_str[pos]==',')			// Si encuentro una coma
			campo++;					// incremento el contador de los campos recibidos del GPS.
		if(campo>2&&campo!=5)			// Selecciono los campos a guardar.
		{
			if(rs_str[pos]!=','&&rs_str[pos]!='.')	// No guardo las comas ni los puntos.
			{
				if(rs_str[pos]=='W'||rs_str[pos]=='E'||rs_str[pos]=='S'||rs_str[pos]=='N')
				{
					if(rs_str[pos]=='W'||rs_str[pos]=='E')
					{
						*armado+=32;	// Codifico el dato que faltó en el proceso anterior.
						armado++;
					}
					*armado++=rs_str[pos];
				}
				else
				{
					if(!(0b00000001&paridad++))			// Controlo si es par o impar.
						*armado=(rs_str[pos]-48)<<4;	// Voy llenando la cadena con los datos obtenidos.
					else
					{
						*armado=rs_str[pos]-48+*armado;	// Voy llenando la cadena con los datos obtenidos.
						*armado=*armado+32;
						armado++;
					}
				}
			}
		}
		pos++;							// Incremento la posición en la cadena auxiliar.
	}while(campo<10);
	*armado++=((ID0<<4)+ID1)+32;		// Cargo el ID del dispositivo.
	*armado++=((ID2<<4)+ID3)+32;
	Info_Extra('E',aux);				// Obtengo el error del GPS.
	for(campo=0;campo<3;campo++)		// Paso el error de ubucación calculado por el GPS.
	{
		*armado++=aux[campo];
	}	
	Info_Extra('V',aux);				// Obtengo la velocidad del GPS.
	for(campo=0;campo<3;campo++)		// Paso la velocidad calculada por el GPS.
	{
		*armado++=aux[campo];
	}
	Altura(aux);
	for(campo=0;campo<4;campo++)		// Paso la altura calculada por el GPS.
	{
		*armado++=aux[campo];
	}
	Info_Extra('D',aux);
	for(campo=0;campo<3;campo++)		// Paso la direccion calculada por el GPS.
	{
		*armado++=aux[campo];
	}
	*armado++='A';						// Lugar en donde voy a informar el estado del disp.
	*armado=0;							// Marco el final de la cadena.
	return 1;							// Lectura correcta de las coordenadas.
}
/********************************************************************************/
/*				OBTENGO LAS COORDENADAS, FHECHA Y HORA							*/
/*..............................................................................*/
void Coordenadas(unsigned char *armado)
{
	static const unsigned char comando[]="$PGRMF";
	unsigned char aux[4];
	volatile unsigned char campo;
	MODO_GPS;							// Habilito el RS232 para el GPS.
	pos=0;
	campo=0;
	control='F';						// Comando que deseo leer.
	DelayMs(2);
	RS232_ON;								// Habilito la interrupcion por RS232.
	while(RCIE)							// Espero que este cargada la lectura del GPS.
	{
		DelayMs(50);
		if(++campo>100)					// tiempo * DelayMs = 5 segundos.
		{
			RS232_OFF;	
			pos=0;
			rs_str[0]=0;
			MODO_PC;
			*armado=0;					// Marco el principio de la cadena con un NULL.
			return;						// Excedió el tiempo de respuesta.
		}
	}
	MODO_PC;							// Dejo de eschuchar el GPS.
	for(pos=0;pos<6;pos++)				// Compruebo el comando recibido.
	{
		if(rs_str[pos]!=comando[pos])
		{
			*armado=0;					// Marco el principio de la cadena con un NULL.
			MODO_PC;
			return;						// No se puede procesar, el comando no es reconocido.
		}
	}
	pos=0;								// Comienzo a escribir en la cadena desde el primer lugar.
	campo=0;
	do									// Almaceno y acomodo el la eeprom los datos recibidos.
	{
		if(rs_str[pos]==',')			// Si encuentro una coma
			campo++;					// incremento el contador de los campos recibidos del GPS.
		if(campo<3||campo==5)			// Selecciono los campos a guardar.
			CLRWDT();
		else
		{
			if(rs_str[pos]==','&&(campo<5||campo==10))
				CLRWDT();
			else
				*armado++=rs_str[pos];	// Voy llenando la cadena con los datos obtenidos.
		}
		pos++;							// Incremento la posición en la cadena auxiliar.
	}while(campo<10);
	*armado=0;							// Marco el final de la cadena.
	return;								// Lectura correcta de las coordenadas.
}
/********************************************************************************/
/*			OBTENGO LA ALTURA CON RESPECTO AL NIVEL DEL MAR.					*/
/*..............................................................................*/
void Altura(unsigned char *valor)
{
	volatile unsigned char campo;
	unsigned char cad_aux[6];
	CLRWDT();
	MODO_GPS;						// Habilito el RS232 para el GPS.
	control='A';					// Comando que deseo leer.
	pos=0;							// Reseteo la cadena.
	campo=0;
	DelayMs(2);
	RS232_ON;							// Habilito la interrupcion por RS232.
	while(RCIE)						// Espero que cargue la cadena recibida.
	{
		DelayMs(50);
		if(++campo>100)				// tiempo * DelayMs = 5 segundos.
		{
			RS232_OFF;
			MODO_PC;
			return;					// Excedió el tiempo de respuesta.
		}
	}
	pos=0;							// Comienzo a escribir en la cadena desde el primer lugar.
	campo=0;
	while(pos<93)					// No excedo el largo de la cadena.
	{
		if(rs_str[pos++]==',')		// Voy contando la cantidad de campos.
			campo++;
		if(campo==9)
			break;					// Salgo del bucle while.
	}
	MODO_PC;						// Dejo de eschuchar el GPS.
	if(rs_str[pos]!=',')			// Hay valores de error?
	{
		cad_aux[0]='0';
		cad_aux[1]='0';
		cad_aux[2]='0';
		cad_aux[3]='0';
		campo=0;
		while(rs_str[pos]!='.'&&rs_str[pos]!=',')	// Busco el punto decimal.
			pos++;
		pos--;						// Vuelvo un lugar.
		campo=3;					// Para el acomodado de la cadena.
		while(rs_str[pos]!=',')
			cad_aux[campo--]=rs_str[pos--];
	}
	else							// No se consiguió un valor válido.
	{
		cad_aux[0]='-';
		cad_aux[1]='-';
		cad_aux[2]='-';
		cad_aux[3]='-';
	}
	rs_str[4]=0;					// Marco el final de la cadena.
	for(campo=0;campo<5;campo++)
	{
		*valor++=cad_aux[campo];	// Paso el valor.
	}
	return;							// Devuelvo el nivel de presicion que tengo.

}
/********************************************************************************/
/*		OBTENGO LA VELOCIDA, ERROR, DIRECCION Y ALTURA DEL GPS					*/
/*..............................................................................*/
unsigned char Info_Extra(unsigned char info,unsigned char *valor)
{
	volatile unsigned char campo;
	unsigned char resultado,lugar;
	CLRWDT();
	MODO_GPS;						// Habilito el RS232 para el GPS.
	if(info=='V')
	{
		control='F';
		lugar=12;
	}
	if(info=='E')
	{
		control='E';
		lugar=1;
	}
	if(info=='D')
	{
		control='F';
		lugar=13;
	}
	pos=0;							// Reseteo la cadena.
	campo=0;
	DelayMs(2);
	RS232_ON;							// Habilito la interrupcion por RS232.
	while(RCIE)						// Espero que cargue la cadena recibida.
	{
		DelayMs(50);
		if(++campo>100)				// tiempo * DelayMs = 5 segundos.
		{
			RS232_OFF;
			MODO_PC;
			return 0;				// Excedió el tiempo de respuesta.
		} 
	}
	pos=0;							// Comienzo a escribir en la cadena desde el primer lugar.
	campo=0;
	while(pos<93)					// No excedo el largo de la cadena.
	{
		if(rs_str[pos++]==',')		// Voy contando la cantidad de campos.
			campo++;
		if(campo==lugar)
			break;					// Salgo del bucle while.
	}
	MODO_PC;						// Dejo de eschuchar el GPS.
	if(rs_str[pos]!=',')			// Hay valores de error?
	{
		rs_str[0]='0';
		rs_str[1]='0';
		rs_str[2]='0';
		campo=0;
		while(rs_str[pos]!='.'&&rs_str[pos]!=',')	// Busco el punto decimal.
			pos++;
		pos--;						// Vuelvo un lugar.
		campo=2;					// Para el acomodado de la cadena.
		while(rs_str[pos]!=',')
			rs_str[campo--]=rs_str[pos--];
		resultado=((rs_str[0]-48)*100)+((rs_str[1]-48)*10)+rs_str[2]-48;
	}
	else							// No se consiguió un valor válido.
	{
		rs_str[0]='-';
		rs_str[1]='-';
		rs_str[2]='-';
		resultado=0;
	}
	rs_str[3]=0;					// Marco el final de la cadena.
	for(campo=0;campo<4;campo++)
	{
		*valor++=rs_str[campo];		// Paso el valor.
	}
	return resultado;				// Devuelvo el nivel de presicion que tengo.
}
/********************************************************************************/
/*				MANDO POR RS232 DOS CARACTERES, CR Y LF.						*/
/*..............................................................................*/
void CR_LF(void)
{
	PutCh(13);						// CR.
	PutCh(10);						// LF.
	return;
}
