/********************************************************************************/
/*		MANEJO DE DOS MODULOS DE MEMORIA 24LC256 CON PAGINACION DE 64 BYTES		*/
/*				Revisión:				1.02									*/
/*				Tipo de comunicación:	I2C										*/
/*				Memoria EEPROM:			24LC256	256 Kbits						*/
/*				Compilador:				MPLAB IDE 8.53 - HI-TECH PICC18 9.50	*/
/*				Tamaño de página:		64 Bytes								*/
/*				Cantidad de módulos:	2										*/
/*				Cantidada de páginas:	1024									*/
/*				Fecha de creación:		19/01/2009								*/
/*				Fecha de modificación:	14/09/2010								*/
/*				Autor:					Mariano Ariel Deville					*/
/********************************************************************************/
	#define PAGINA	64
#if defined(LC256)
	#define PAGINAS	1024
#endif
#if defined(LC512)
	#define PAGINAS	2048
#endif
/********************************************************************************/
/*						PROTOTIPO DE FUNCIONES									*/
/*..............................................................................*/
void Write_24LC(unsigned int pagina,unsigned char dato);
unsigned char Read_24LC(unsigned int pagina);
void Guardo_Mem_Aux(const unsigned char *datos);
void Rec_Paquete_24LC(unsigned int mem,unsigned char *dato);
unsigned int Recupero_Mem_Aux(unsigned char *datos);
void Borrar_24LC256(unsigned int pagina);
/********************************************************************************/
/*			GUARDO EN LA 24LC256 UN BYTE UTILIZANDO I2C							*/
/*..............................................................................*/
void Write_24LC(unsigned int direccion,unsigned char dato)
{
	unsigned char high_byte,low_byte,chip_select;
	CLRWDT();
	chip_select=0b00000000;				// Selecciono por defecto el primier chip de memoria.
	if(direccion>32767)					// Calculo en que chip de memoria almaceno.
	{
		chip_select=0b00000010;			// Selecciono el segundo chip de memoria.
		direccion-=32768;				// Obtengo la nueva dirección para este chip.
	}
	low_byte=direccion&0xff;			// Obtengo la parte baja de la dir.
	high_byte=(direccion>>8)&0x7f;		// Obtengo la parte alta de la dir.
	I2C_Start();						// Comienzo la comunicación I2C.
	I2C_Write(0b10100000|chip_select);	// Envio el byte de control.
	I2C_Write(high_byte);				// Escribo la parte alta de la direccion de memoria.
	I2C_Write(low_byte);				// Escribo la parte baja de la direccion de memoria.
	I2C_Write(dato);					// Escribo el dato en memoria.
	I2C_Stop();							// Termino la comunicación I2C.
	DelayMs(5);							// 
	return;
}
/********************************************************************************/
/*			RECUPERO DE LA 24LC256 UN BYTE UTILIZANDO I2C						*/
/*..............................................................................*/
unsigned char Read_24LC(unsigned int direccion)
{
	unsigned char high_byte,low_byte,dato,chip_select;
	CLRWDT();
	chip_select=0b00000000;				// Selecciono por defecto el primier chip de memoria.
	if(direccion>32767)					// Calculo en que chip de memoria almaceno.
	{
		chip_select=0b00000010;			// Selecciono el segundo chip de memoria.
		direccion-=32768;				// Obtengo la nueva dirección para este chip.
	}
	low_byte=direccion&0xff;			// Obtengo la parte baja de la dir.
	high_byte=(direccion>>8)&0x7f;		// Obtengo la parte alta de la dir.
	I2C_Start();						// Comienzo la cominicación I2C.
	I2C_Write(0b10100000|chip_select);	// Envio el byte de control.
	I2C_Write(high_byte);				// Escribo la parte alta de la direccion de memoria.
	I2C_Write(low_byte);				// Escribo la parte baja de la direccion de memoria.
	I2C_RepStart();						// Reinicio la comunicación I2C.
	I2C_Write(0b10100001|chip_select);	// Envio el byte de control.
	dato=I2C_Read(0);					// Obtengo el dato guardado.
	I2C_Stop();							// Termino la comunicación I2C.
	return(dato);						// Devuelvo el valor leido.
}
/********************************************************************************/
/*			RECUPERO DE LA MEMORIA 24LC256 UNA CADENA DE CARACTERES				*/
/*..............................................................................*/
void Rec_Paquete_24LC(unsigned int pagina,unsigned char *dato)
{
	unsigned int dir;
	if(pagina>(PAGINA-1))			// Excedo la capacidad de la memoria.
		return;
	dir=pagina*PAGINA;				// Calculo la dirección de la memoria.
	do
	{
		(*dato)=Read_24LC(dir++);	// Obtengo el valor almacenado en la mem.
		if(dir>=(pagina+1)*PAGINA)	// No me excedo de la pagina que debo leer.
		{
			*dato=0;				// Le agrego el final de cadena que no tiene.
			return;					//
		}
	}while(*dato++);				// Sigo hasta encontrar la marca de final (NULL).
	*dato=0;						// Me aceguro el NULL
	return;
}
/********************************************************************************/
/*			ALMACENO LOS DATOS QUE NO HE PODIDO ENVIAR POR GPRS					*/
/*..............................................................................*/
void Guardo_Mem_Aux(const unsigned char *datos)
{
	unsigned char dato;				// Buffer de lectura.
	unsigned int dir=0,pag=0;		// Direccion de memmoria y pagina inicial.
	pos=0;
	do								// Busco el último lugar libre.
	{
		dato=Read_24LC(pag*PAGINA);	// Cargo en un buffer el contenido de la memoria.
		if(dato!=26)				// Es distinto a lugar disponible.
			pag++;					// Voy a la posición que sigue.
	}while(dato!=26&&pag<=(PAGINAS-1));	// Lo hago mientras haya algo en la memoria (512Kb).
	if(pag==(PAGINAS-1))
		Write_24LC(0,26);			// Marco el siguiente lugar libre.
	if(pag>=(PAGINAS-1))			// No encontre marca o estoy en el últimmo lugar?
		pag=0;						// Escribo en la pagina cero.
	Write_24LC((pag+1)*PAGINA,26);	// Marco el siguiente lugar libre.
	dir=pag*PAGINA;					// Direccion de la pagina libre.
	pos=0;
	do								// Guardo la cadena.
	{
		dato=*datos++;				// Barro la cadena.
		Write_24LC(dir++,dato);		// Recorro la pagina escribiendo la cadena.
	}while(dato&&dir<((pag+1)*PAGINA));	// Mientras tenga algo que grabar y no exceda la mem.
	Write_24LC(dir,0);				// Marco con un cero el final de la cadena.
	return;							// Devuelvo la pagina en la que escribi.
}
/********************************************************************************/
/*			RECUPERO UNA PAGINA DESDE LA MEMORIA EEPROM PARA ENVIARLA.			*/
/*..............................................................................*/
unsigned int Recupero_Mem_Aux(unsigned char *datos)
{
	unsigned char dato;
	unsigned int dir,pag=0;			// Direccion de memmoria y pagina.
	do								// Busco el primer lugar ocupado.
	{
		dato=Read_24LC(pag*PAGINA);	// Cargo en un buffer el contenido de la memoria.
		if(dato<32)					// La pagina esta vacia?
			pag++;					// Voy a la posición que sigue.
		if(pag>=(PAGINAS-1)||envio)	// Termine de recorrer toda la memoria.
			return 10000;			// Salgo porque no encontre nada.
	}while(dato<32);				// Recorro mientras no encuentre nada.
	dir=pag*PAGINA;					// Direccion de la pagina libre.
	pos=0;
	do								// Cargo el dato encontrado en la eeprom del PIC.
	{
		if(envio)
			return 10000;			// Salgo, es tiempo de transmitir.
		dato=Read_24LC(dir++);		// Recupero un byte de la memoria auxiliar.
		if(dato>31)
			*datos++=dato;			// Guardo el byte en la memoria del 
	}while(dato&&pos++<=(PAGINA-1));	// Mientras tenga algo que grabar y no exceda la mem.
	*datos=0;						// Marco con un cero el final de la cadena.
	return pag;						// Devuelvo el valor de la pagina 
}
/********************************************************************************/
/*			BORRO TODO EL CONTENIDO DE UNA PAGINA DE LA MEMORIA 24LC256.		*/
/*..............................................................................*/
void Borrar_24LC256(unsigned int pagina)
{
	Write_24LC((pagina*PAGINA),0);	// Escribo un cero al principio de la pagina.
	return;
}
