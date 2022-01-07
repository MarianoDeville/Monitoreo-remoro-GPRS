/********************************************************************************/
/*					MENU AUXILIAR PARA APOYO AL INSTALADOR						*/
/*				Revisión:				1.00									*/
/*				PIC:					PIC18F4620								*/
/*				MODEM:					SIM340CZ								*/
/*				GPS:					GPS15H									*/
/*				Memoria EEPROM:			24LC256									*/
/*				Teclado:				PS2										*/
/*				Dispaly:				C-51505 2 lineas por 20 caracteres.		*/
/*				Puerto RS232:			1 para comunicación externa.			*/
/*				Compilador:				MPLAB IDE 8.53 - HI-TECH PICC18 9.50	*/
/*				Fecha de creación:		29/06/2010								*/
/*				Fecha de finalización:	29/09/2010								*/
/*				Autor:					Mariano Ariel Deville					*/
/********************************************************************************/
/*						PROTOTIPOS DE FUNCIONES									*/
/*------------------------------------------------------------------------------*/
void Menu_Servicio(void);
void Comandos(void);
/********************************************************************************/
/*			CAPTURA DE TECLADO Y VISUALIZACION EN DISPLAY						*/
/*------------------------------------------------------------------------------*/
void Menu_Servicio(void)
{
	unsigned char espera;
	Lcd_Setup();
	Imprimir_Lcd("* * N Y X M O N * *","* * N Y X W A V E * *",1);
	espera=0;							// Reseteo el tiempo de espera.
	Reseteo_Teclado();
	while(INT0IE)						// Espero un enter del teclado
	{									// para procesar el comando.
		if(espera++>200)				// Tiempo agotado???
		{
			INT0IE=0;					// Deshabilito la interrupción por teclado PS2.
			return;						// Regreso sin entrar al menú de servicio.
		}
		DelayMs(10);					// Tiempo de espera = Delay * espera
	}
	consola=1;							// Levanta la bandera para el menu de servicio.
	Linea_Lcd("Menu de servicio",1);	// 
	borrar=0;							// Queda en display hasta que presiono una tecla.
	TMR1IF=0;							// Bajo la bandera de la interrupción TMR1.
	TMR1IE=0;							// Deshabilito la interrupcion por TMR1.
	MODEM_RESET=0;
	while(consola)						// Mientras este el jumper puesto sigo en este menu.
	{
		Reseteo_Teclado();
		while(INT0IE)					// Espero un enter del teclado
		{								// para procesar el comando.
			if(pos_str_ps2||borrar)		// Actulizo la pantalla?
			{
				Linea_Lcd(rs_str,1);	// Muestro lo que se va escribiendo.
				borrar=1;				// Bandera para borrar el display y actualizar.
			}
			DelayMs(50);				// Para el refresco.
		}
		Comandos();						// Busco y ejecuto el comando ingresado.
		MODO_PC;
		CLRWDT();
	}
	Lcd_Clear();
	INT0IE=0;							// Deshabilito la interrupción por teclado PS2.
	TMR1H=0b00001011;					// Configuro el tiempo que tarda en generar
	TMR1L=0b11011011;					// la interrupcion.
	TMR1IF=0;							// Bajo la bandera de la interrupción TMR1.
	TMR1IE=1;							// Habilito la interrupcion por TMR1.
	return;
}
/****************************************************************************/
/*		COMANDOS PARA CONFIGURAR GPS, MODEM, MEMORIA Y SENSORES				*/
/*--------------------------------------------------------------------------*/
/*			SETUP GENERAL													*/
/*			COORDENADAS 4800												*/
/*			COORDENADAS?													*/
/*			GPS ERROR?														*/
/*			VELOCIDAD?														*/
/*			AUTOLOCATE														*/
/*			GPS RESET														*/
/*			GPS CONFIG														*/
/*			CONFIG 4800 A 9600												*/
/*			DESCARGAR EEPROM												*/
/*			INFO PRODUCTO													*/
/*			BORRAR EEPROM													*/
/*			SENSORES														*/
/*			SENSOR X														*/
/*			DESCARGAR MEMORIA												*/
/*			BARRIDO MEMORIA													*/
/*			BORRAR MEMORIA													*/
/*			BORRADO COMPLETO												*/
/*			MODEM SETUP														*/
/*			CONECTAR MODEM													*/
/*			APAGAR MODEM													*/
/*			INICIALIZAR GPRS												*/
/*			COMANDOS AT														*/
/*			Version soft? y hard?											*/
/****************************************************************************/
void Comandos(void)
{
	volatile unsigned int pag;
	volatile unsigned char i;
	unsigned char cache,emp_tel,cad_aux[80];
	bis=0;
	if(!rs_str[0])
	{
		borrar=1;								// Borro el display.
		return;
	}
	if(Comparo_Cadenas(rs_str,"SETUP GENERAL"))	// Muestro en pantalla las coordenadas.
	{
		Linea_Lcd("Configurando GPS...",1);
		Serial_Setup(4800);						// Seteo el puerto serie en 4800 baudios.
		Comandos_GPS('C');						// Seteo el GPS con la informacion que necesito.
		Serial_Setup(9600);						// Seteo el puerto serie en 9600 baudios.
		Linea_Lcd("Configurando EEPROM...",1);
		Setup_Eeprom();							//
		Linea_Lcd("Borrando memoria...",1);				
		for(pag=0;pag<=65534;pag++)				// 
			Write_24LC(pag,0);					// Escribo un cero al principio de la pagina.
		Linea_Lcd("Configurando MODEM...",1);
		MODO_MODEM;								// Habilito el RS232 para el MODEM.
		POWER_MODEM=0;							// Prendo el MODEM.
		while(!NET_STATE)						// Espero que el MODEM me de una señal.
		{
			if(cache++>100)						// Timeout???
			{
				LED=1;							// Dejo el led fijo prendido.
				POWER_MODEM=1;					// Apago el MODEM.
				Linea_Lcd("No se puede prender el MODEM",1);
				borrar=0;						// Queda en display hasta que presiono una tecla.
				return;
			}
			DelayMs(30);
			LED=!LED;							// Parpadea el led mientras se prende el MODEM.
		}
		LED=1;									// Dejo el led fijo prendido.
		POWER_MODEM=1;							// Prendo el MODEM.
		DelayS(1);
		Modem_Setup();							// Configuracion inicial del MODEM.
		borrar=0;								// Queda en display hasta que presiono una tecla.
		Modem_Power(0,0);						// Apago el MODEM.
		Imprimir_Lcd("Configuracion","terminado.",1);
		borrar=0;								// Queda en display hasta que presiono una tecla.
		return;
	}
	if(Comparo_Cadenas(rs_str,"COORDENADAS 4800"))	// Muestro en pantalla las coordenadas.
	{
		Serial_Setup(4800);						// Seteo el puerto serie en 4800 baudios.
		bis=1;									// Entro en el proximo "if" tambien.
	}
	if(Comparo_Cadenas(rs_str,"COORDENADAS?")||bis)	// Muestro en pantalla las coordenadas.
	{
		if(Leo_Coordenadas(cad_aux))			// Obtengo y almaceno las coordenadas del GPS.
		{
			Lcd_Clear();						// Limpio display.
			Lcd_Goto(0);						// Voy al comienzo de la primer linea.
			pos=13;								// Me posiciono en el comienzo de los datos que me interesan.
			Lcd_Puts("Lat:   ");
			while(pos<24)						// Barro el valor de la latitud.
				Lcd_Putch(cad_aux[pos++]);		// Imprmimo en el display el valor leido.
			Lcd_Goto(0x40);						// Voy al comienzo de la segunda linea.
			pos++;								// Me posiciono en el comienzo de los datos que me interesan.
			Lcd_Puts("Long: ");
			while(pos<37)						// Barro el valor de la longitud.
				Lcd_Putch(cad_aux[pos++]);		// Imprmimo en el display el valor leido.
		}
		else
			Imprimir_Lcd("Error al obtener","las coordenadas",1);
		borrar=0;								// Queda en display hasta que presiono una tecla.
		Serial_Setup(9600);						// Seteo el puerto serie en 9600 baudios.
		return;									// Vuelvo a esperar otro comando.
	}
	if(Comparo_Cadenas(rs_str,"GPS ERROR?"))	// Obtengo el error de posicionamiento.
	{

		Info_Extra('E',cad_aux);
		Linea_Lcd(cad_aux,1);
		borrar=0;								// Queda en display hasta que presiono una tecla.
		return;
	}
	if(Comparo_Cadenas(rs_str,"VELOCIDAD?"))	// Obtengo la velocidad de movimiento.
	{

		Info_Extra('V',cad_aux);
		Linea_Lcd(cad_aux,1);
		borrar=0;								// Queda en display hasta que presiono una tecla.
		return;
	}
	if(Comparo_Cadenas(rs_str,"AUTOLOCATE"))
	{
		Comandos_GPS('A');
		borrar=0;								// Queda en display hasta que presiono una tecla.
		return;									// Vuelvo a esperar otro comando.
	}
	if(Comparo_Cadenas(rs_str,"GPS RESET"))
	{
		Comandos_GPS('R');
		borrar=0;								// Queda en display hasta que presiono una tecla.
		return;									// Vuelvo a esperar otro comando.
	}
	if(Comparo_Cadenas(rs_str,"CONFIG 9600"))
	{
		Serial_Setup(4800);						// Seteo el puerto serie en 4800 baudios.
		bis=1;									// Entro en el proximo "if" tambien.
	}
	if(Comparo_Cadenas(rs_str,"GPS CONFIG")||bis)	// Configuro el GPS.
	{
		Comandos_GPS('C');
		Serial_Setup(9600);						// Seteo el puerto serie en 9600 baudios.
		borrar=0;								// Queda en display hasta que presiono una tecla.
		return;									// Vuelvo a esperar otro comando.
	}
	if(Comparo_Cadenas(rs_str,"DESCARGAR EEPROM"))	// Envio por RS232info y configuración.
	{
		Linea_Lcd("Descargando...",1);
		Descargar_Memoria();					// Mando los datos de la EEPROM por puerto serie.
		PutCh(13);								// Mando un ENTER al finalizar.
		Linea_Lcd("Memoria descargada.",1);
		borrar=0;								// Queda en display hasta que presiono una tecla.
		return;
	}
	if(Comparo_Cadenas(rs_str,"INFO?"))			// Muestro por display info y configuración.
	{
		CLRWDT();
		for(pag=0;pag<20;pag++)
		{
			cad_aux[pag]=Eeprom_Read(pag);
		}
		Lcd_Clear();							// Limpio display.
		Lcd_Goto(0);							// Voy al comienzo de la primer linea.
		for(i=0;i<10;i++)
		{
			Lcd_Write(cad_aux[i]);
		}
		Lcd_Goto(0x40);							// Voy al comienzo de la segunda linea.
		Lcd_Puts("FDM");
		i=cad_aux[10]/10;						// Calculo las decenas.
		Lcd_Write(i);							// Imprimo por display las decenas.
		Lcd_Write(cad_aux[10]-i);				// Imprimo por display las unidad.
		Lcd_Puts(" minutos.");
		borrar=0;								// Queda en display hasta que presiono una tecla.
		return;
	}
	if(Comparo_Cadenas(rs_str,"BORRAR EEPROM"))	// Limpiar el contenido de la memoria EEPROM.
	{
		Borrar_Memoria();						// Borro todo el contenido de la EEPROM del PIC.
		Linea_Lcd("Memoria borrada.",1);
		borrar=0;								// Queda en display hasta que presiono una tecla.
		return;
	}
	if(Comparo_Cadenas(rs_str,"SENSORES")||bis)	// Muestro en pantalla las lecturas de los sensores.
	{
		Linea_Lcd("Cantidad de sensores:",1);
		Lcd_Putch(cant_sens+48);				// Imprimo en display la cantidad de sensores conectados.
		Espero_Enter();							// Espero que presionen la tecla ENTER del teclado PS2.
		i=0;									// Comienzo el barrido desde el primero.
		while(i<7)								// 
		{
			CLRWDT();		
			valor_adc=Medir_ADC(i++,5);			// Obtengo la temperatura de uno de los sensores.
			IntToStr(valor_adc,rs_str);
			Lcd_Clear();						// Limpio el display.
			Lcd_Goto(0x00);						// Inicio de linea en el display.
			Lcd_Puts("Med. sensor ");			// 
			Lcd_Putch(i+48);					// Imprimo el numero de sensor.
			Lcd_Putch(':');						// 
			Lcd_Goto(0x40);						// Voy al comienzo de la segunda linea.
			Lcd_Puts(rs_str);					// Imprimo el valor de temperatura medido.
			Espero_Enter();						// Espero que presionen la tecla ENTER del teclado PS2.
		}
		borrar=1;								// Bandera para borrar el display y actualizar.
		return;
	}
	for(i=0;i<=5;i++)
	{
		cad_aux[i]=rs_str[i];
	}
	cad_aux[6]=0;
	if(Comparo_Cadenas(cad_aux,"SENSOR"))		// Muestro en pantalla la lectura del sensor.
	{
		if(rs_str[7]<48||rs_str[7]>57)
			i=0;
		else
			i=rs_str[7]-48;
		Reseteo_Teclado();
		while(1)								// 
		{
			CLRWDT();		
			valor_adc=Medir_ADC(i++,5);			// Obtengo la temperatura de uno de los sensores.
			IntToStr(valor_adc,rs_str);
			Lcd_Clear();						// Limpio el display.
			Lcd_Goto(0x00);						// Inicio de linea en el display.
			Lcd_Puts("Med. sensor ");			// 
			Lcd_Putch(i+48);					// Imprimo el numero de sensor.
			Lcd_Putch(':');						// 
			Lcd_Goto(0x40);						// Voy al comienzo de la segunda linea.
			rs_str[0]=' ';						// No imprimo la coma.
			Lcd_Puts(rs_str);					// Imprimo el valor de temperatura medido.
			DelayMs(100);
			if(!INT0IE)							// Espero un enter del teclado
				break;
		}
		borrar=1;								// Bandera para borrar el display y actualizar.
		return;
	}
	if(Comparo_Cadenas(rs_str,"DESCARGAR MEMORIA"))	// Mando el contenido de la memoria externa por RS232.
	{
		Imprimir_Lcd("","descargando...",0);
		do
		{
			pag=Recupero_Mem_Aux(rs_str);
			if(pag<5000)						// La página tiene algún contenido?
			{
				Borrar_24LC256(pag);
				PutStr(rs_str);					// La mando por RS232.
				PutCh(10);						//
				PutCh(13);						// Enter.
			}
		}while(pag<5000);
		Imprimir_Lcd("Memoria descargada.","",1);
		borrar=0;								// Queda en display hasta que presiono una tecla.
		return;
	}
	if(Comparo_Cadenas(rs_str,"BARRIDO MEMORIA"))	// Mando el contenido de la memoria externa por RS232.
	{
		Imprimir_Lcd("","descargando...",0);
		for(pag=0;pag<=PAGINAS;pag++)			// Barro las 512 paginas de las memorias.
		{
			Rec_Paquete_24LC(pag,rs_str);		// Cargo en memoria la página.
			if(rs_str[0])						// La página tiene algún contenido?
			{
				PutStr(rs_str);					// La mando por RS232.
				rs_str[0]=0;
				PutCh(10);						//
				PutCh(13);						// Enter.
			}
		}
		Imprimir_Lcd("Memoria descargada.","",1);
		borrar=0;								// Queda en display hasta que presiono una tecla.
		return;
	}
	if(Comparo_Cadenas(rs_str,"BORRAR MEMORIA"))	// Borrado rápido de la memoria.
	{
		Imprimir_Lcd("","borrando...",0);
		for(pag=0;pag<=512;pag++)				// Barro las 512 paginas de las memorias.
			Borrar_24LC256(pag);				// Borro pagina por pagina.
		Imprimir_Lcd("Memoria borrada.","",1);
		borrar=0;								// Queda en display hasta que presiono una tecla.
		return;
	}
	if(Comparo_Cadenas(rs_str,"BORRADO COMPLETO"))	// Borrado lento, barro cada uno de los bytes.
	{
		Imprimir_Lcd("","borrando...",0);
		for(pag=0;pag<=65534;pag++)				// 
			Write_24LC(pag,0);					// Escribo un cero al principio de la pagina.
		Linea_Lcd("Memoria borrada.",1);
		borrar=0;								// Queda en display hasta que presiono una tecla.
		return;
	}
	if(Comparo_Cadenas(rs_str,"MODEM SETUP"))	// Configuracion inicial del MODEM.
	{
		MODO_MODEM;								// Habilito el RS232 para el MODEM.
		POWER_MODEM=0;							// Prendo el MODEM.
		while(!NET_STATE)						// Espero que el MODEM me de una señal.
		{
			if(cache++>100)						// Timeout???
			{
				LED=1;							// Dejo el led fijo prendido.
				POWER_MODEM=1;					// Apago el MODEM.
				Linea_Lcd("No se puede prender el MODEM",1);
				borrar=0;						// Queda en display hasta que presiono una tecla.
				return;
			}
			DelayMs(30);
			LED=!LED;							// Parpadea el led mientras se prende el MODEM.
		}
		LED=1;									// Dejo el led fijo prendido.
		POWER_MODEM=1;							// Prendo el MODEM.
		Imprimir_Lcd("","Configurando MODEM...",0);
		DelayS(1);
		Modem_Setup();							// Configuracion inicial del MODEM.
		Imprimir_Lcd("MODEM configurado","",1);	// 
		borrar=0;								// Queda en display hasta que presiono una tecla.
		Modem_Power(0,0);						// Apago el MODEM.
		return;
	}
	if(Comparo_Cadenas(rs_str,"CONECTAR MODEM"))	// Inicializo el MODEM.
	{
		Imprimir_Lcd("","inicializando.....",0);
		emp_tel=Modem_Power(1,4);				// Prendo el MODEM y obtengo el operador.
		if(!emp_tel)							// Encontre operador celular?
			emp_tel=Modem_Power(1,0);			// Intento en otra banda..
		if(emp_tel)								// Encontre operador???
			Imprimir_Lcd("MODEM encendido.","",1);
		else
			Imprimir_Lcd("Operador no encontrado.","",1);
		borrar=0;								// Queda en display hasta que presiono una tecla.
		return;
	}
	if(Comparo_Cadenas(rs_str,"APAGAR MODEM"))	// Inicializo el MODEM.
	{
		if(Modem_Power(0,0))					// 
			Imprimir_Lcd("MODEM apagado","",1);
		else
			Imprimir_Lcd("error al apagar","el MODEM",1);
		borrar=0;								// Queda en display hasta que presiono una tecla.
		return;
	}
	if(Comparo_Cadenas(rs_str,"INICIALIZAR GPRS"))	// Inicializo el GPRS del MODEM.
	{
		if(!emp_tel)
		{
			Imprimir_Lcd("No hay proveedor de","telefonia disponible.",1);
			borrar=0;
			return;
		}
		Imprimir_Lcd("Configurando...","",1);
		if(Inicializo_GPRS(emp_tel))			// Configuro el GPRS según la empresa telefónica que sea.
			Imprimir_Lcd("GPRS configurado","",1);
		else
			Imprimir_Lcd("Error al configurar","el GPRS del MODEM",1);
		borrar=0;								// Queda en display hasta que presiono una tecla.
		return;
	}
	if(Comparo_Cadenas(rs_str,"COMANDOS AT"))	// Entro directamente al MODEM con comandos por teclado.
	{
		cache=0;								// Reseteo el contador que uso para el timeout.
		MODO_MODEM;								// Habilito el RS232 para el MODEM.
		POWER_MODEM=0;							// Prendo el MODEM.
		while(!NET_STATE)						// Espero una señal del MODEM.
		{
			if(cache++>100)						// Timeout???
			{
				LED=1;							// Dejo el led fijo prendido.
				POWER_MODEM=1;
				Linea_Lcd("No se puede prender el MODEM",1);
				borrar=0;						// Queda en display hasta que presiono una tecla.
				return;
			}
			DelayMs(30);
			LED=!LED;							// Parpadea el led mientras se prende el MODEM.
		}
		LED=1;									// Dejo el led fijo prendido.
		POWER_MODEM=1;							// Prendo el MODEM.
		Linea_Lcd("Ingrese comando AT:",1);
		while(1)								// Espero comando de salida.
		{
			Reseteo_Teclado();
			borrar=0;
			while(INT0IE)						// Espero un enter del teclado
			{									// para procesar el comando.
				if(pos_str_ps2||borrar)			// Actulizo la pantalla?
				{
					Linea_Lcd(rs_str,1);		// Muestro lo que se va escribiendo.
					borrar=1;					// Bandera para borrar el display y actualizar.
				}
				DelayMs(50);					// Para el refresco.
			}
			if(Comparo_Cadenas(rs_str,"SALIR"))	// Comando para salir.
			{
				PutStr("AT+CPOWD=1");			// Apago el MODEM.
				PutCh(13);						// Mando un ENTER.
				DelayS(1);
				borrar=1;						// Bandera para borrar el display y actualizar.
				return;
			}
			Consulta_Modem(rs_str,rs_str);		// Envio el comando escrito por teclado PS2.
			Linea_Lcd(rs_str,1);				// Imprimo la respuesta del MODEM.
		}
	}
	if(Comparo_Cadenas(rs_str,"Version soft? y hard?"))	// 
	{
		Imprimir_Lcd("NYXMON REV.1.00","28/09/2010",1);
		Espero_Enter();							// Espero que presionen la tecla ENTER del teclado PS2.
		Imprimir_Lcd("PIC18F4620","",1);
		Espero_Enter();							// Espero que presionen la tecla ENTER del teclado PS2.
		Imprimir_Lcd("AUTOR:","MARIANO A. DEVILLE",1);
		borrar=0;								// Queda en display hasta que presiono una tecla.
	}
	if(Comparo_Cadenas(rs_str,"SALIR"))			// 
	{
		consola=0;								// Bajo bandera para salir del menú de servicio.
		Lcd_Clear();
		return;
	}
	Imprimir_Lcd("Comando inexistente.","",1);	// Si llegue hasta aca es porque no existe el comando.
	borrar=0;									// Queda en display hasta que presiono una tecla.
	return;
}


