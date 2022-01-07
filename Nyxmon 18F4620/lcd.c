/********************************************************************************/
/*				MANEJO DE DISPLAY CON INTERFACE DE 4 BITS						*/
/********************************************************************************/
static bit LCD_RS	@ ((unsigned)&PORTB*8+2);	// Register select
static bit LCD_EN	@ ((unsigned)&PORTB*8+3);	// Enable

void Lcd_Write(unsigned char);
void Lcd_Clear(void);
void Lcd_Puts(const char * s);
void Lcd_Goto(unsigned char pos);
void Lcd_Setup(void);
void Lcd_Putch(char c);
void Imprimir_Lcd(const char *LINEA_1,const char *LINEA_2,unsigned char BORRAR);
void Linea_Lcd(const char *linea,unsigned char borrar);

#define	LCD_STROBE	(LCD_EN=1);(LCD_EN=0)

/********************************************************************************/
/*					*/
/********************************************************************************/
void Lcd_Write(unsigned char c)
{
	CLRWDT();
	PORTB &=0x0F;
	PORTB |=c&0xF0;
	LCD_STROBE;
	PORTB &=0x0F;
	PORTB |=(c << 4)&0xF0;
	LCD_STROBE;
	DelayUs(40);
}
/********************************************************************************/
/*					*/
/********************************************************************************/
void Lcd_Clear(void)
{
	LCD_RS=0;
	Lcd_Write(0x1);
	DelayMs(2);
}
/********************************************************************************/
/*					*/
/********************************************************************************/
void Lcd_Puts(const char *s)
{
	LCD_RS=1;				// write characters
	while(*s)
		Lcd_Write(*s++);
}
/********************************************************************************/
/*					*/
/********************************************************************************/
void Lcd_Goto(unsigned char pos)
{
	LCD_RS=0;
	Lcd_Write(0x80+pos);
}
/********************************************************************************/
/*					*/
/********************************************************************************/
void Lcd_Setup(void)
{
/************************************************************************/
/* 		initialise the LCD - put into 4 bit mode 						*/
/*		movlw		b'00001111'		;LCD on, Cursor On, Parpadeo On		*/
/*		movlw		b'00001110'		;LCD on, Cursor On, Parpadeo Off	*/
/*		movlw		b'00001101'		;LCD on, Cursor Off, Parpadeo On	*/
/*		movlw		b'00001100'		;LCD on, cursor off. Parpadeo OFF	*/
/************************************************************************/
	LCD_RS=0;				// write control bytes
	DelayMs(15);			// power on delay
	PORTB &=0x0F;			// -- Clear the port
	PORTB |=0B00110000;		// Es para que salga el dato por RB7,RB6,RB5,RB4
	LCD_STROBE;
	DelayMs(5);
	LCD_STROBE;
 	DelayUs(100);
	LCD_STROBE;
	DelayMs(5);
	PORTB &=0x0F;			// -- Clear the port
	PORTB |=0B00100000;
	LCD_STROBE;
	DelayUs(40);
	Lcd_Write(0x28);		// 4 bit mode, 1/16 duty, 5x8 font
	DelayUs(40);
	Lcd_Write(0b00001100);	// configuro la visualización.
}
/********************************************************************************/
/*					*/
/********************************************************************************/
void Lcd_Putch(char c)
{
	char str[2];
	str[0]=c;
	str[1]=0;
	Lcd_Puts(str);
	return;
}
/********************************************************************************/
/*					*/
/********************************************************************************/
void Imprimir_Lcd(const char *linea_1,const char *linea_2,unsigned char borrar)
{
	if(borrar)
		Lcd_Clear();
	Lcd_Goto(0x00);
	Lcd_Puts(linea_1);
	Lcd_Goto(0x40);
	Lcd_Puts(linea_2);
	return;
}
/********************************************************************************/
/*					*/
/********************************************************************************/
void Linea_Lcd(const char *linea,unsigned char borrar)
{
	unsigned char largo=0;
	if(borrar)
		Lcd_Clear();
	Lcd_Goto(0x00);
	while(*linea)
	{
		Lcd_Putch(*linea++);
		largo++;
		if(largo==20)
			Lcd_Goto(0x40);
	}
	return;
}
