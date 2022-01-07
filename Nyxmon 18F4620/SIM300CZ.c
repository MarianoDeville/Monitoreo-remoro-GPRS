/********************************************************************************/
/*	MANEJO DEL MODEM SIM340C MEDIANTE COMANDOS AT ENVIADOS POR PUERTO SERIE.	*/
/*				Revisión:				1.00									*/
/*				PIC:					PIC18F4620								*/
/*				MODEM:					SIM340CZ								*/
/*				Compilador:				MPLAB IDE 8.53 - HI-TECH PICC18 9.50	*/
/*				Fecha de creación:		29/06/2010								*/
/*				Fecha de modificación:	29/09/2010								*/
/*				Autor:					Mariano Ariel Deville					*/
/********************************************************************************/
/*						Prototipo de funciones.									*/
/*..............................................................................*/
unsigned char Modem_Power(unsigned char accion,unsigned char banda);
void Modem_Setup(void);
void Consulta_Modem(const char *comando_at,unsigned char *respuesta);
unsigned char Inicializo_GPRS(unsigned char emp_tel);
unsigned char Espero_Ok(void);
unsigned char Mando_Paquete_TCP(const unsigned char *datos);
unsigned char Proveedor(void);
unsigned char Envio_Informe(const unsigned char *datos);
unsigned char Envio_Buffer(const unsigned char *datos,unsigned char tipo);
void Aviso_Problema(unsigned char codigo);
unsigned char Modem_Banda(unsigned char banda);
/********************************************************************************/
/*			Manejo la alimentacion del MODEM y controlo que arranque.			*/
/*..............................................................................*/
unsigned char Modem_Power(unsigned char accion,unsigned char banda)
{
	const static unsigned char comando[]="Call Ready";
	unsigned char tiempo,tiempo1,c_banda,esperando;
	c_banda=0;
	esperando=0;
	CLRWDT();
	if(accion)						// Debo prender el MODEM??
	{
		MODO_MODEM;					// Habilito el RS232 para el MODEM.
		DelayMs(90);
		POWER_MODEM=0;				// Enciendo el modem
		tiempo=0;
		while(!NET_STATE)			// Espero que el MODEM encienda.
		{
			if(tiempo++>60)			// Espero 3 segundos.
			{
				POWER_MODEM=1;		// 
				LED=1;				// Dejo el LED prendido.
				MODO_PC;
				return 0;
			}
			DelayMs(50);			// Junto con el contador tiempo doy el timeout.
			LED=!LED;				// Parpadea el LED mientras espera que arranque.
		}
		LED=1;						// Dejo el LED prendido.
		POWER_MODEM=1;				// 
		tiempo1=0;					// Reseteo el contador para timeout.
		while(!esperando)
		{
			CLRWDT();
			pos=0;					// Reseteo la cadena.
			tiempo=0;				// Reseteo el contador para timeout.
			RS232_ON;				// Habilito la interrupción por RS232.
			while(RCIE)				// Espero que termine la recepción RS232.
			{
				DelayMs(240);
				if(tiempo++>250)	// tiempo * DelayMs = 60 segundos.
				{
					RS232_OFF;
					rs_str[pos]=0;
					return 0;
				}
			}
			if(!c_banda)
			{
				Modem_Banda(banda);	// Configuro en que banda quiero que se conecte.
				c_banda=1;			// Solo entro una vez.
			}
			esperando=1;			// Los caracteres coinciden.
			for(pos=0;pos<11;pos++)	// Compruebo el comando recibido.
			{
				if(rs_str[pos]!=comando[pos])	// Algun caracter es distinto?
					esperando=0;	// Bajo la bandera.
			}
			if(tiempo1++>6)			// No repito el ciclo mas de 6 veces.
				return 0;
		}
		tiempo=Proveedor();			// Configuro el prooveedor de telefonía
		return tiempo;				// Devuelvo quien es el proveedor, 0= no encontrado.
	}
	else							// Entonces lo apago.
	{
		if(rst++>3||banda)			// Corto la alimentación del MODEM?
		{
			MODEM_RESET=1;			// Apago el MODEM.
			rst=0;
		}
		POWER_MODEM=0;
		tiempo=0;
		while(1)					// Espero que el MODEM encienda.
		{
			if(tiempo++>20)			// Espero 2 segundos.
			{
				MODEM_RESET=0;		// Alimento el MODEM (pero sigue apagado)
				POWER_MODEM=1;
				MODO_PC;
				return 1;
			}
			DelayMs(100);			// Junto con el contador tiempo doy el timeout.
		}
	}
	return 1;						// Apagado de MODEM correcto.
}
/********************************************************************************/
/*		CONFIGURACION INICIAL DEL MODEM, SE ACCEDE DESDE MENU DE SERVICIO.		*/
/*..............................................................................*/
void Modem_Setup(void)
{
	CLRWDT();
	MODO_MODEM;						// Me asegro de estar escuchando al MODEM.
	DelayMs(90);
	PutStr("AT");					// Abro consola.
	PutCh(13);
	DelayMs(100);
	PutStr("AT");					// 
	Espero_Ok();
	PutStr("ATE0");					// Desactivo el eco de los comandos.
	Espero_Ok();
	PutStr("AT+IPR=9600");			// Seteo el puerto del modem a 9600 bps.
	Espero_Ok();
	PutStr("AT&W");					// Guardo la configuracion en el MODEM.
	Espero_Ok();
}
/********************************************************************************/
/*			CONSULTO EL MODEM Y GUARDO LA RESPUESTA EN UNA CADENA.				*/
/*..............................................................................*/
void Consulta_Modem(const char *comando_at,unsigned char *respuesta)
{
	unsigned char tiempo=0;
	CLRWDT();
	pos=0;							// Reseteo la cadena.
	PutStr(comando_at);				// Envio el comando AT al modem
	RS232_ON;						// Habilito la interrupcion por RS232.
	PutCh(13);						// Enter para que se ejecute el comando.
	while(RCIE)						// Espero que cargue la cadena recibida.
	{
		DelayMs(40);
		if(tiempo++>250)			// tiempo * DelayMs = 10 segundos.
		{
			RS232_OFF;
			rs_str[pos]=0;
			return;					// Excedió el tiempo de respuesta.
		}
	}
	pos=0;							// Reseteo el contador para barrer la cadena desde el principio.
	while(rs_str[pos]&&pos<93)		// Mientras no sea un final de cadena.
		*respuesta++=rs_str[pos++];	// Copio la respuesta en la cadena de destino.
	*respuesta=0;					// marco el final de la cadena.
	return;
}
/********************************************************************************/
/*			CONFIGURO EL GPRS SEGUN EL PROVEEDOR DE TELEFONIA					*/
/*..............................................................................*/
unsigned char Inicializo_GPRS(unsigned char emp_tel)
{
	CLRWDT();
	PutStr("AT+CGDCONT=1,\"IP\",\"");
	if(emp_tel=='P')
		PutStr("gprs.personal.com\"");
	if(emp_tel=='C')
		PutStr("internet.ctimovil.com.ar\"");
	if(emp_tel=='U')
		PutStr("internet.gprs.unifon.com.ar\"");
	if(!Espero_Ok())					// Espero respuesta del MODEM.
		return 0;						// No se pudo realizar la configuración.
	PutStr("AT+CSTT=\"");
	if(emp_tel=='P')
		PutStr("gprs.personal.com\",\"gprs\",\"gprs\"");
	if(emp_tel=='C')
		PutStr("internet.ctimovil.com.ar\",\"Ctigprs\",\"ctigprs99\"");
	if(emp_tel=='U')
		PutStr("internet.gprs.unifon.com.ar\",\"gprs\",\"gprs\"");
	if(!Espero_Ok())					// Espero respuesta del MODEM.
		return 0;						// No se pudo realizar la configuración.
	PutStr("AT+CIICR");
	if(!Espero_Ok())					// Espero respuesta del MODEM.
		return 0;						// No se pudo realizar la configuración.
	Consulta_Modem("AT+CIFSR",rs_str);	// Obtengo IP.
	return 1;							// Configuracion realizada.
}
/********************************************************************************/
/*					ESPERO LA RESPUESTA DEL MODEM								*/
/*..............................................................................*/
unsigned char Espero_Ok(void)
{
	unsigned char tiempo_ok=0;
	CLRWDT();
	pos=0;								// Reseteo la cadena.
	RS232_ON;							// Habilito la interrupción por RS232.
	PutCh(13);							// Le doy enter al comando.
	while(RCIE)
	{
		DelayMs(40);
		if(tiempo_ok++>250)				// tiempo * DelayMs = 10 segundos.
		{
			RS232_OFF;
			rs_str[pos]=0;
			return 0;					// Excedió el tiempo de respuesta.
		}
	}
	return 1;
}
/********************************************************************************/
/*				 ENVIO UN PAQUETE DE DATOS USANDO TCP POR GPRS					*/
/*..............................................................................*/
unsigned char Mando_Paquete_TCP(const unsigned char *datos)
{
	unsigned char dato,temp_m;
	unsigned char str_aux[60];			// Cadena auxiliar de uso múltiple.
	CLRWDT();
	dato=*datos;						// Leo el primer caracter a enviar.
	if(dato<=31||dato>=81)				// Caracter no valido para enviar???
		return 1;						// Descarto el paquete.
	PutStr("AT+CIPSTART=\"TCP\",\"200.81.0.207\",1234");
	Espero_Ok();						// Espero el OK del comando.
	DelayMs(100);
	Espero_Ok();						// Espero el estado de conexion OK.
	PutStr("AT+CIPSEND");				// Abro el envío.
	Espero_Ok();						// Espero ">" (terminal).
	pos=0;
	do									// Envio la cadena.		
	{
		dato=*datos++;					// Barro la cadena.
		if(pos<6)						// Guardo en "linea_uno" la palabra de control.
			str_aux[pos]=dato;
		if(dato>31)						// No mando NULL.
			PutCh(dato);				// Mando por RS232 el caracter leido.
	}while(dato&&pos++<127);			// Lo hago hasta encontrar un final de cadena.
	PutCh(10);							// Marco el final del mensaje.
	PutCh(26);							// CTRL Z fin del mensaje.
	Espero_Ok();						// Espero respuesta del MODEM.
	str_aux[6]=0;						// Marco el final de la cadena.
	pos=0;
	temp_m=0;
	RS232_ON;							// Habilito la interrupcion por RS232.
	while(RCIE)							// Espero a terminar de cargar la respuesta.
	{
		DelayMs(120);
		if(temp_m++>250)				// tiempo * DelayMs = 30 segundos.
		{
			PutStr("AT+CIPCLOSE");		// Cierro la conexión GPRS.
			PutCh(13);
			DelayS(3);
			RS232_OFF;
			rs_str[pos]=0;
			return 0;					// Salgo porque excedió el tiempo de respuesta.
		}
	}
	pos=0;
	do									// Compruebo el comando recibido.
	{
		if(rs_str[pos]!=str_aux[pos])
		{
			PutStr("AT+CIPCLOSE");		// Cierro la conexión GPRS.
			PutCh(13);
			DelayS(3);
			return 0;					// No se puede procesar, el comando no es reconocido.
		}
	}while(pos++<5);
	temp_m=0;
	pos++;
	while(pos<11)						// Busco si hay algún comando para ejecutar.
	{
		if(rs_str[pos++]==',')			// Encontre otro campo?
		{
			do							// Obtengo el comando.
			{
				str_aux[temp_m++]=rs_str[pos++];
			}while((rs_str[pos]>31)&&pos<60);
			str_aux[temp_m]=0;			// Marco final de cadena.
			Ejecutar_Comando(str_aux);	// La funcion esta en el bloque principal.
		}
	}	
	PutStr("AT+CIPCLOSE");				// Cierro la conexión GPRS.
	PutCh(13);
	DelayS(3);
	return 1;							// Datos enviados con exito al servidor.
}
/********************************************************************************/
/*			Espero a tener señal e identificar el proveedor de telefonia.		*/
/*..............................................................................*/
unsigned char Proveedor(void)
{
	static bit encontrado;
	unsigned char temp2,temp1,timeout;
	static const unsigned char per[]="PERSONAL",cti[]="CTI Movil",uni[]="UNIFON";
	CLRWDT();
	temp2=0;
	while(temp2++<10)					// Intento durante 60 segundos.
	{
		DelayS(5);						// Cada cinco segundo intento de nuevo.
		temp1=0;						// Inicializo las variables.
		pos=0;
		PutStr("AT+COPS?");				// Buscando proveedor. 
		RS232_ON;						// Habilito la interrupcion por RS232.
		PutCh(13);						// Le doy el enter al comando.
		while(RCIE)						// Espero a terminar de cargar la respuesta.
		{
			DelayMs(20);
			if(temp1++>100)				// tiempo * DelayMs = 2 segundos.
			{
				RS232_OFF;
				rs_str[pos]=0;
				pos=0;
				return 0;				// Excedió el tiempo de respuesta.
			}
		}
		pos=0;							// Inicializo las variables.
		timeout=0;
		while(rs_str[pos++]!='"'&&pos<20)	// Busco el nombre del proveedor.
		{
			if(timeout++>90)
				break;
		}
		encontrado=1;
		temp1=0;
		timeout=0;
		if(rs_str[pos]=='P')			// Personal???
		{
			while(rs_str[pos]!='"'&&timeout++<70)	// Comparo la respuesta.
			{
				if(rs_str[pos++]!=per[temp1++])
					encontrado=0;		// No coincide la empresa.
			}
			if(encontrado)				// Coincidio la cadena de respuesta?
				return 'P';				// confirmado, es personal.
		}
		if(rs_str[pos]=='C')			// CTI???
		{
			while(rs_str[pos]!='"'&&timeout++<70)	// Comparo la respuesta.
			{
				if(rs_str[pos++]!=cti[temp1++])
					encontrado=0;		// No coincide la empresa.
			}
			if(encontrado)				// Coincidio la cadena de respuesta?
				return 'C';				// confirmado, es cti.
		}
		if(rs_str[pos]=='U')			// Unifon???
		{
			while(rs_str[pos]!='"'&&timeout++<70)	// Comparo la respuesta.
			{
				if(rs_str[pos++]!=uni[temp1++])
					encontrado=0;		// No coincide la empresa.
			}
			if(encontrado)				// Coincidio la cadena de respuesta?
				return 'U';				// confirmado, es unifon.
		}
	}
	return 0;							// Proveedor no reconocido.
}
/********************************************************************************/
/*				ENVIO UN PAQUETE DE DATOS POR GPRS								*/
/*..............................................................................*/
unsigned char Envio_Informe(const unsigned char *datos)
{
	unsigned char config_emp;
	static bit estado;
	CLRWDT();
	config_emp=Modem_Power(1,4);		// Enciendo el MODEM y obtengo la empresa.
	if(!config_emp)						// Encontre empresa??
	{
		Modem_Power(0,1);				// Bajo la alimentacion del MODEM.
		DelayS(5);
		config_emp=Modem_Power(1,0);	// Enciendo el MODEM y obtengo la empresa.
	}
	if(!config_emp)						// Encontre empresa??
	{
		Modem_Power(0,1);				// Apago el MODEM.
		return 0;						// Aviso sobre el error.
	}
	Inicializo_GPRS(config_emp);		// Configuro el GPRS para el proveedor.
	DelayS(1);
	estado=Mando_Paquete_TCP(datos);	// 
	Modem_Power(0,0);					// Apago el MODEM.
	return estado;						// Operacion realizada con excito.
}
/********************************************************************************/
/*	ENVIO DEL CONTENIO EN LA MEMORIA (ENVIOS MULTIPLES SIN APAGAR EL MODEM)		*/
/*..............................................................................*/
unsigned char Envio_Buffer(const unsigned char *datos,unsigned char tipo)
{
	unsigned char config_emp;
	static bit estado;
	CLRWDT();
	if(tipo==1)							// Debo prender el MODEM?
	{
		config_emp=Modem_Power(1,4);	// Enciendo el MODEM y obtengo la empresa.
		if(!config_emp)					// Encontre empresa??
		{
			Modem_Power(0,1);			// Bajo la alimentacion del MODEM.
			DelayS(5);
			config_emp=Modem_Power(1,0);	// Enciendo el MODEM y obtengo la empresa.
		}
		if(!config_emp)					// Encontre empresa??
		{
			Modem_Power(0,1);			// Apago el MODEM.
			return 0;					// Aviso sobre el error.
		}
	}
	else
		config_emp=Proveedor();			// Confirmo el proveedor.
	if(!config_emp)
	{
		Modem_Power(0,0);				// Apago el MODEM.
		return 0;						// Operacion realizada con excito.
	}
	Inicializo_GPRS(config_emp);		// Configuro el GPRS para el proveedor.
	estado=Mando_Paquete_TCP(datos);	// 
	if(!estado)
	{
		Modem_Power(0,0);				// Apago el MODEM.
		return estado;					// Operacion realizada con excito.
	}
	return tipo+1;						// Operacion realizada con excito.
}
/********************************************************************************/
/*				ENVIO DE SMS CON INFORME SOBRE ERRORES							*/
/*..............................................................................*/
void Aviso_Problema(unsigned char codigo)
{
	CLRWDT();
	Modem_Power(1,4);					// Enciendo el MODEM.
	DelayS(5);
	PutStr("AT+CMGF=1");
	Espero_Ok();						// Espero la respuesta del MODEM.
	PutStr("AT+CSCS=\"GSM\"");			// Configuro para enviar msn.
	Espero_Ok();						// Espero la respuesta del MODEM.
//	PutStr("AT+CMGS=\"3515101890\"");	// Roberto.
//	PutStr("AT+CMGS=\"3515332186\"");	// Antonio.
	PutStr("AT+CMGS=\"3516207722\"");	// Mariano (yo).
	Espero_Ok();						// Espero la respuesta del MODEM.
	if(!codigo)							// Que debo informar????
		PutStr("ERROR 0 - Error al obtener las coordenadas del GPS.");
	if(codigo==1)						// Que debo informar????
		PutStr("ERROR 1 - El formato del paquete es incorrecto.");
	PutCh(26);							// CTRL Z fin del mensaje.
	Espero_Ok();						// Espero la respuesta del MODEM.
	Modem_Power(0,0);					// Apago el MODEM.
	return;
}
/********************************************************************************/
/*					HAGO UN CAMBIO DE BANDA.									*/
/*..............................................................................*/
unsigned char Modem_Banda(unsigned char banda)
{
	CLRWDT();
	if(banda>=5)
		return 0;
	PutStr("AT+CBAND=\"");
	if(!banda)
		PutStr("PGSM_MODE\"");
	if(banda==1)
		PutStr("DCS_MODE\"");
	if(banda==2)
		PutStr("PCS_MODE\"");
	if(banda==3)
		PutStr("EGSM_DCS_MODE\"");
	if(banda==4)
		PutStr("GSM850_PCS_MODE\"");
	Espero_Ok();						// Espero la respuesta del cambio de banda.
	return 1;
}
