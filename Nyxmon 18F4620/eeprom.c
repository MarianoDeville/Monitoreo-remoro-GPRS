/********************************************************************************/
/*			MANEJO DE LA MEMÓRIA EEPROM DEL PIC 18F452							*/
/*				Revisión:				1.00									*/
/*				PIC:					PIC18F4620								*/
/*				Compilador:				MPLAB IDE 8.53 - HI-TECH PICC18 9.50	*/
/*				Fecha de creación:		29/06/2010								*/
/*				Fecha de modificación:	15/09/2010								*/
/*				Autor:					Mariano Ariel Deville					*/
/********************************************************************************/
/*						PROTOTIPOS DE FUNCIONES									*/
/*..............................................................................*/
void Setup_Eeprom(void);
unsigned char Eeprom_Read(unsigned char address);
void Eeprom_Write(unsigned char address,unsigned char value);
void Descargar_Memoria(void);
void Borrar_Memoria(void);
/********************************************************************************/
/*		ARMADO DE LOS DATOS EN LA EEPROM LA PRIMERA VEZ QUE ARRANCA EL PIC.		*/
/*..............................................................................*/
void Setup_Eeprom(void)
{
	CLRWDT();
	Borrar_Memoria();		// Borro todo el contenido de la memoria.
	Eeprom_Write(0,'N');	// ID del producto.
	Eeprom_Write(1,'Y');
	Eeprom_Write(2,'X');
	Eeprom_Write(3,'M');
	Eeprom_Write(4,'O');
	Eeprom_Write(5,'N');
	Eeprom_Write(6,ID0);	
	Eeprom_Write(7,ID1);
	Eeprom_Write(8,ID2);
	Eeprom_Write(9,ID3);
	Eeprom_Write(10,3);		// Minutos entre envios.
	Eeprom_Write(11,90);	// Velocidad máxima.
	Eeprom_Write(12,5);		// Cantidad de sensores analógicos.
/*
	Eeprom_Write(13,0);		// 
	Eeprom_Write(14,0);		// 
	Eeprom_Write(15,0);		// 
	Eeprom_Write(16,0);		// 
	Eeprom_Write(17,0);		// 
	Eeprom_Write(18,0);		// 
	Eeprom_Write(19,0);		// 
	Eeprom_Write(20,0);		// 
	Eeprom_Write(21,0);		// 
	Eeprom_Write(22,0);		// 
	Eeprom_Write(23,0);		// 
	Eeprom_Write(24,0);		// 
	Eeprom_Write(25,0);		// 
	Eeprom_Write(26,0);		// 
*/
	return;
}
/********************************************************************************/
/*		DEVUELVO EL BYTE LEIDO EN LA DIRECCIÓN PASADA COMO ARGUMENTO			*/
/*..............................................................................*/
unsigned char Eeprom_Read(unsigned char address)
{
	CLRWDT();
	FREE=0;
	EEADR=address;				// Paso la dirección en la que voy a leer.
	EEPGD=0;					// Point to EE memory
	CFGS=0;
	RD=1;						// Inicio el ciclo de lectura.
	while(RD);					// Espero a que termine la lectura.
	return EEDATA;				// Devuelvo el valor leido.
}
/********************************************************************************/
/*				ESCRIBO UN BYTE EN LA MEMORIA EEPROM							*/
/*..............................................................................*/
void Eeprom_Write(unsigned char address,unsigned char value)
{
 	while(WR);					// Me aseguro que no haya una escritura en proceso.
	CLRWDT();
	FREE=0;
	EEADR=address;				// Paso la dirección en la que voy a leer.
	EEDATA=value; 				// Paso el byte que deseo escribir.
	EEPGD=0;
	CFGS=0;
 	WREN=1;						// EEPROM modo escritura.
	EECON2=0x55;
	EECON2=0xAA;
	WR=1;						// Escribo en memoria.
 	while(WR);					// Espero que termine de escribir.
	WREN=0;						// Deshabilito EEPROM modo escritura.
	return;
}
/********************************************************************************/
/*				MANDO TODO EL CONTENIDO DE LA MEMORIA							*/
/*..............................................................................*/
void Descargar_Memoria(void)
{
	volatile unsigned int i;
	unsigned char dato;
	for(i=0;i<1024;i++)			// Recorro los 1024 bytes de memoria.
	{
		dato=Eeprom_Read(i);	// Leo el contenido de la memoria.
		PutCh(dato);			// Mando por puerto serie el caracter leido.
	}
	return;
}
/********************************************************************************/
/*					ESCRIBO EN TODA LA MEMRIA NULL								*/
/*..............................................................................*/
void Borrar_Memoria(void)
{
	volatile unsigned int i;
	for(i=0;i<1023;i++)			// Recorro los 256 bytes de memoria.
		Eeprom_Write(i,0);		// Escribo un cero en la memoria.
	return;
}
