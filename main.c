#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "util/delay.h"

#include "DIO_interface.h"
#include "CLCD_interface.h"
#include "DIO_reg.h"

void Calc_voidClearResult(void);

void main (void)
{
	/* Set the port A as output */
	DIO_u8SetPortDirection(DIO_u8PORTA, DIO_u8PORT_OUTPUT);

	/* Activate the internal pull up resistor */
	DIO_u8SetPinDirection(DIO_u8PORTD, DIO_u8PIN2, DIO_u8PIN_INPUT);
	DIO_u8SetPinDirection(DIO_u8PORTD, DIO_u8PIN3, DIO_u8PIN_INPUT);
	DIO_u8SetPinDirection(DIO_u8PORTD, DIO_u8PIN4, DIO_u8PIN_INPUT);


	/* Set the switch pin as input */
	DIO_u8SetPinValue(DIO_u8PORTD, DIO_u8PIN2, DIO_u8PIN_HIGH);
	DIO_u8SetPinValue(DIO_u8PORTD, DIO_u8PIN3, DIO_u8PIN_HIGH);
	DIO_u8SetPinValue(DIO_u8PORTD, DIO_u8PIN4, DIO_u8PIN_HIGH);


	/* The pattern of arrow */
	uint8 Local_au8Pattern[8] = {0b00000100, 0b00001110, 0b0010101, 0b00000100, 0b00000100, 0b00000100, 0b00000100, 0b00000000};


	/* The array of first number (0 -> 9) */
	uint8 Local_au8ArrayFirstNum[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

	/* The array of first number (1 -> 9) */
	uint8 Local_au8ArraySecondNum[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

	/* The array of sum, sub, multiply and divide signs */
	char Local_achArraySign[4] = {'+','-', '*', '/'};

	/* Initialization of the LCD (4Bit) */
	CLCD_voidInit();

	sint8 Local_s8FirstNumIter = 0;
	sint8 Local_s8SecondNumIter = 0;
	sint8 Local_s8Sign = 0;
	sint8 Local_u8ArrowIterator = 0;

	/* Display the program */
	CLCD_voidGoToXY(0,0);
	CLCD_voidSendNumber(Local_au8ArrayFirstNum[Local_s8FirstNumIter]);

	CLCD_voidGoToXY(1,0);
	CLCD_voidSendData(Local_achArraySign[Local_s8Sign]);

	CLCD_voidGoToXY(2,0);
	CLCD_voidSendNumber(Local_au8ArraySecondNum[Local_s8SecondNumIter]);

	CLCD_voidGoToXY(3,0);
	CLCD_voidSendData('=');

	CLCD_u8SendSpecialCharacter(0, Local_au8Pattern, Local_u8ArrowIterator, 1);


	while(1)
	{
		if(GET_BIT(PIND, DIO_u8PIN4) == DIO_u8PIN_LOW) // SW4 (Enter Switch) is pressed
		{
			_delay_ms(100);
			if((GET_BIT(PIND, DIO_u8PIN4)) == DIO_u8PIN_LOW) // For the Debouncing
			{
				if(Local_u8ArrowIterator == 3)
				{
					if(GET_BIT(PIND, DIO_u8PIN4) == DIO_u8PIN_LOW) // For the Debouncing
					{
						_delay_ms(30);
						if((GET_BIT(PIND, DIO_u8PIN4)) == DIO_u8PIN_LOW) // For the Debouncing
						{
							if(Local_s8Sign == 0) 	   //Summation operation
							{
								CLCD_voidGoToXY(4,0);
								CLCD_voidSendNumber(Local_au8ArrayFirstNum[Local_s8FirstNumIter] + Local_au8ArraySecondNum[Local_s8SecondNumIter]);
							}
							else if(Local_s8Sign == 1) // Subtraction operation
							{
								CLCD_voidGoToXY(4,0);
								CLCD_voidSendNumber(Local_au8ArrayFirstNum[Local_s8FirstNumIter] - Local_au8ArraySecondNum[Local_s8SecondNumIter]);
							}
							else if(Local_s8Sign == 2) // Multiplication operation
							{
								CLCD_voidGoToXY(4,0);
								CLCD_voidSendNumber(Local_au8ArrayFirstNum[Local_s8FirstNumIter] * Local_au8ArraySecondNum[Local_s8SecondNumIter]);
							}
							else if(Local_s8Sign == 3) // Divide operation
							{
								CLCD_voidGoToXY(4,0);
								/* Give the integer  number => 1.54 --> 1 */
								sint8 Local_u32Result2 = Local_au8ArrayFirstNum[Local_s8FirstNumIter] / Local_au8ArraySecondNum[Local_s8SecondNumIter];

								/* Give the numbers after the comma => 1.54 --> .45 */
								f32 Local_f32Result = (((f32)(Local_au8ArrayFirstNum[Local_s8FirstNumIter]) / (f32)(Local_au8ArraySecondNum[Local_s8SecondNumIter])) - Local_u32Result2) * 100;

								CLCD_voidGoToXY(4,0);
								CLCD_voidSendNumber(Local_u32Result2);
								CLCD_voidGoToXY(5, 0);
								CLCD_voidSendData('.');
								CLCD_voidGoToXY(6, 0);
								CLCD_voidSendNumber(Local_f32Result);
							}
							while((GET_BIT(PIND, DIO_u8PIN4)) == DIO_u8PIN_LOW);
						}
					}
				}

				if(Local_u8ArrowIterator == 4)
				{
					/* When the arrow be at position (4,1) after equal sign, clear the result and move the arrow to the begin */
					Calc_voidClearResult();
					Local_u8ArrowIterator = -1;
					CLCD_u8SendSpecialCharacter(0, Local_au8Pattern, Local_u8ArrowIterator, 1);
				}
				CLCD_voidGoToXY(Local_u8ArrowIterator,1);
				CLCD_voidSendData(' ');
				Local_u8ArrowIterator++;
				CLCD_u8SendSpecialCharacter(0, Local_au8Pattern, Local_u8ArrowIterator, 1);
			}
			while(GET_BIT(PIND, DIO_u8PIN4) == DIO_u8PIN_LOW);
		}

		if(Local_u8ArrowIterator == 0)
		{
			if(GET_BIT(PIND, DIO_u8PIN2) == DIO_u8PIN_LOW) // SW2 (Up Switch) is pressed
			{
				_delay_ms(100);
				if((GET_BIT(PIND, DIO_u8PIN2)) == DIO_u8PIN_LOW) // For the Debouncing
				{
					Local_s8FirstNumIter++;
					if(Local_s8FirstNumIter == 10)
					/* When arrive to last element in the array, start from the first element */
					{
						Local_s8FirstNumIter = 0;
					}

					CLCD_voidGoToXY(0,0);
					CLCD_voidSendNumber(Local_au8ArrayFirstNum[Local_s8FirstNumIter]);
				}
				while(GET_BIT(PIND, DIO_u8PIN2) == DIO_u8PIN_LOW);

			}
			else if(GET_BIT(PIND, DIO_u8PIN3) == DIO_u8PIN_LOW) // SW3 (Down Switch) is pressed
			{
				_delay_ms(100);
				if((GET_BIT(PIND, DIO_u8PIN3)) == DIO_u8PIN_LOW) // For the Debouncing
				{
					Local_s8FirstNumIter--;

					if(Local_s8FirstNumIter == -1)
					/* When arrive to first element in the array, start from the last element */
					{
						Local_s8FirstNumIter = 9;
					}

					CLCD_voidGoToXY(0,0);
					CLCD_voidSendNumber(Local_au8ArrayFirstNum[Local_s8FirstNumIter]);
				}
				while(GET_BIT(PIND, DIO_u8PIN3) == DIO_u8PIN_LOW);
			}
		}
		else if(Local_u8ArrowIterator == 1)
		{
			if(GET_BIT(PIND, DIO_u8PIN2) == DIO_u8PIN_LOW) // SW2 (Up Switch) is pressed
			{
				_delay_ms(100);
				if((GET_BIT(PIND, DIO_u8PIN2)) == DIO_u8PIN_LOW) // For the Debouncing
				{
					Local_s8Sign++;
					if(Local_s8Sign == 4)
					/* When arrive to last element in the array, start from the first element */
					{
						Local_s8Sign = 0;
					}

					CLCD_voidGoToXY(1,0);
					CLCD_voidSendData(Local_achArraySign[Local_s8Sign]);
				}
				while(GET_BIT(PIND, DIO_u8PIN2) == DIO_u8PIN_LOW);
			}

			else if(GET_BIT(PIND, DIO_u8PIN3) == DIO_u8PIN_LOW) // SW3 (Down Switch) is pressed
			{
				_delay_ms(100);
				if(GET_BIT(PIND, DIO_u8PIN3) == DIO_u8PIN_LOW) // For the Debouncing
				{
					Local_s8Sign--;
					if(Local_s8Sign == -1)
					/* When arrive to first element in the array, start from the last element */
					{
						Local_s8Sign = 3;
					}

					CLCD_voidGoToXY(1,0);
					CLCD_voidSendData(Local_achArraySign[Local_s8Sign]);
				}
				while(GET_BIT(PIND, DIO_u8PIN3) == DIO_u8PIN_LOW);
			}
		}
		else if(Local_u8ArrowIterator == 2)
		{
			if(GET_BIT(PIND, DIO_u8PIN2) == DIO_u8PIN_LOW) // SW2 (Up Switch) is pressed
			{
				_delay_ms(100);
				if((GET_BIT(PIND, DIO_u8PIN2)) == DIO_u8PIN_LOW) // For the Debouncing
				{
					Local_s8SecondNumIter++;
					if(Local_s8SecondNumIter == 9)
					/* When arrive to last element in the array, start from the first element */
					{
						Local_s8SecondNumIter = 0;
					}

					CLCD_voidGoToXY(2,0);
					CLCD_voidSendNumber(Local_au8ArraySecondNum[Local_s8SecondNumIter]);
				}
				while((GET_BIT(PIND, DIO_u8PIN2)) == DIO_u8PIN_LOW);

			}
			else if(GET_BIT(PIND, DIO_u8PIN3) == DIO_u8PIN_LOW) // SW3 (Down Switch) is pressed
			{
				_delay_ms(100);
				if((GET_BIT(PIND, DIO_u8PIN3)) == DIO_u8PIN_LOW) // For the Debouncing
				{
					Local_s8SecondNumIter--;
					if(Local_s8SecondNumIter == -1)
					/* When arrive to first element in the array, start from the last element */
					{
						Local_s8SecondNumIter = 8;
					}

					CLCD_voidGoToXY(2,0);
					CLCD_voidSendNumber(Local_au8ArraySecondNum[Local_s8SecondNumIter]);
				}
				while((GET_BIT(PIND, DIO_u8PIN3)) == DIO_u8PIN_LOW);

			}
		}
	}
}


void Calc_voidClearResult(void)
{
	CLCD_voidGoToXY(4, 1);
	CLCD_voidSendData(' ');

	CLCD_voidGoToXY(4, 0);
	CLCD_voidSendData(' ');

	CLCD_voidGoToXY(5, 0);
	CLCD_voidSendData(' ');

	CLCD_voidGoToXY(6, 0);
	CLCD_voidSendData(' ');

	CLCD_voidGoToXY(7, 0);
	CLCD_voidSendData(' ');
}





