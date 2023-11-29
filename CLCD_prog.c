#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "ErrType.h"
#include <util/delay.h>

#include "DIO_interface.h"

#include "CLCD_interface.h"
#include "CLCD_cfg.h"
#include "CLCD_prv.h"

/********************************************************************************************/
/********************************************************************************************/
#if CLCD_u8BIT_MODE == FOUR_BIT
static void voidSetHalfDataPort(uint8 Copy_u8Data)
{
	DIO_u8SetPinValue(CLCD_u8DATA_PORT, CLCD_u8D4_PIN, GET_BIT(Copy_u8Data,0));
	DIO_u8SetPinValue(CLCD_u8DATA_PORT, CLCD_u8D5_PIN, GET_BIT(Copy_u8Data,1));
	DIO_u8SetPinValue(CLCD_u8DATA_PORT, CLCD_u8D6_PIN, GET_BIT(Copy_u8Data,2));
	DIO_u8SetPinValue(CLCD_u8DATA_PORT, CLCD_u8D7_PIN, GET_BIT(Copy_u8Data,3));
}
#endif
/********************************************************************************************/
/********************************************************************************************/
static void voidSendEnablePulse(void)
{
	/* Send enable pulse */
	DIO_u8SetPinValue(CLCD_u8CTRL_PORT, CLCD_u8E_PIN, DIO_u8PIN_HIGH);
	_delay_ms(2);
	DIO_u8SetPinValue(CLCD_u8CTRL_PORT, CLCD_u8E_PIN, DIO_u8PIN_LOW);
}
/********************************************************************************************/
/********************************************************************************************/
void CLCD_voidSendCmd(uint8 Copy_u8Cmd)
{
	/* Set RS pin to low for command */
	DIO_u8SetPinValue(CLCD_u8CTRL_PORT, CLCD_u8RS_PIN, DIO_u8PIN_LOW);

	/* Set RW pin to low for writing */
#if CLCD_u8RW_CONN_STS == DIO_CONNECTED
	DIO_u8SetPinValue(CLCD_u8CTRL_PORT, CLCD_u8RW_PIN, DIO_u8PIN_LOW);
#endif

#if CLCD_u8BIT_MODE == EIGHT_BIT
	/* Send the command */
	DIO_u8SetPortValue(CLCD_u8DATA_PORT, Copy_u8Cmd);
	voidSendEnablePulse();

#elif CLCD_u8BIT_MODE == FOUR_BIT
	/* Send the 4 most significant bits of the command first */
	voidSetHalfDataPort(Copy_u8Cmd >> 4);
	voidSendEnablePulse();
	voidSetHalfDataPort(Copy_u8Cmd);
	voidSendEnablePulse();

#else
#error Wrong CLCD_u8BIT_MODE configuration option
#endif
}
/********************************************************************************************/
/********************************************************************************************/
void CLCD_voidSendData(uint8 Copy_u8Data)
{
	/* Set RS to high for data */
	DIO_u8SetPinValue(CLCD_u8CTRL_PORT, CLCD_u8RS_PIN, DIO_u8PIN_HIGH);

	/* Set RW pin to low for writing */
#if CLCD_u8RW_CONN_STS == DIO_CONNECTED
	DIO_u8SetPinValue(CLCD_u8CTRL_PORT, CLCD_u8RW_PIN, DIO_u8PIN_LOW);
#endif

#if CLCD_u8BIT_MODE == EIGHT_BIT
	/* Send the data */
	DIO_u8SetPortValue(CLCD_u8DATA_PORT, Copy_u8Data);
	voidSendEnablePulse();

#elif CLCD_u8BIT_MODE == FOUR_BIT
	/* Send the 4 most significant bits of the data first */
	voidSetHalfDataPort(Copy_u8Data >> 4);
	voidSendEnablePulse();
	voidSetHalfDataPort(Copy_u8Data);
	voidSendEnablePulse();

#else
#error Wrong CLCD_u8BIT_MODE configuration option
#endif
}
/********************************************************************************************/
/********************************************************************************************/
void CLCD_voidInit(void)
{
	/* Wait for more than 30ms after power on */
	_delay_ms(40);

#if CLCD_u8BIT_MODE == EIGHT_BIT
	/* Function set command : 2 lines, Font size (5 * 7) */
	CLCD_voidSendCmd(0b00111000);

#elif CLCD_u8BIT_MODE == FOUR_BIT
	voidSetHalfDataPort(0b0010);
	voidSendEnablePulse();
	voidSetHalfDataPort(0b0010);
	voidSendEnablePulse();
	voidSetHalfDataPort(0b1000);
	voidSendEnablePulse();

#endif

	/* Display ON/OFF control command : Display on, Cursor off, Blinking off */
	CLCD_voidSendCmd(0b00001100);

	/* Display clear */
	CLCD_voidSendCmd(1);
}
/********************************************************************************************/
/********************************************************************************************/
uint8 CLCD_u8SendString(const char* Copy_pchString)
/* Char not uint8 or sint8 because it not unsigned or signed it only characters.
 * Const to avoid modifying the string by user inside the function.
 */
{
	uint8 Local_u8ErrorState = OK;
	if(Copy_pchString != NULL)
	{
		uint8 Local_u8Iterator = 0u;
		while(Copy_pchString[Local_u8Iterator] != '\0')
		{
			CLCD_voidSendData(Copy_pchString[Local_u8Iterator]);
			Local_u8Iterator++;
		}
	}
	else
	{
		Local_u8ErrorState = NULL_PTR_ERR;
	}

	return Local_u8ErrorState;
}
/********************************************************************************************/
/********************************************************************************************/
void CLCD_voidSendNumber(sint32 Copy_s32Number)
{
	uint8 Local_u8RightDigit;
	char Local_chNum[10];
	uint8 Local_u8Counter1 = 0;
	sint8 Local_u8Counter2;


	if(Copy_s32Number == 0)
	{
		CLCD_voidSendData('0');
		return;
	}
	else if(Copy_s32Number < 0)
	{
		/* Number is -ve, make it +ve and print the -ve sign on the LCD */
		CLCD_voidSendData('-');
		Copy_s32Number *= -1;
	}

	while(Copy_s32Number != 0)
	{
		/* Get the right most digit */
		Local_u8RightDigit = (uint8)((uint32)Copy_s32Number % 10);

		/* Remove the right digit */
		Copy_s32Number /= 10;

		/* Convert the right digit to the array and convert it to ASCII value */
		Local_chNum[Local_u8Counter1] = Local_u8RightDigit + '0';

		Local_u8Counter1++;
	}

	for(Local_u8Counter2 = (sint8)Local_u8Counter1 - 1; Local_u8Counter2 >= 0; Local_u8Counter2--)
	{
		CLCD_voidSendData(Local_chNum[(uint8)Local_u8Counter2]);
	}
}
/********************************************************************************************/
/********************************************************************************************/
void CLCD_voidGoToXY(uint8 Copy_u8XPos, uint8 Copy_u8YPos)
{
	uint8 Local_u8Address;
	if(Copy_u8YPos == 0u)
	{
		Local_u8Address = Copy_u8XPos;
	}
	else if(Copy_u8YPos == 1u)
	{
		Local_u8Address = 0x40 + Copy_u8XPos;
	}
	/* Local_u8Address = (0x40 * Copy_u8YPos) + Copy_u8XPos */

	/* Set bit 7 for SetDDRAM Address command */
	SET_BIT(Local_u8Address,7);

	/* Execute Set DDRAM Address command */
	CLCD_voidSendCmd(Local_u8Address);
}
/********************************************************************************************/
/********************************************************************************************/
uint8 CLCD_u8SendSpecialCharacter(uint8 Copy_LocationNum, uint8* Copy_pu8Pattern, uint8 Copy_u8XPos, uint8 Copy_u8YPos)
{
	uint8 Local_u8ErrorState = OK;
	if(Copy_pu8Pattern != NULL)
	{
		uint8 Local_u8CGRAMAddress = Copy_LocationNum * 8;
		uint8 Local_u8Counter =  0u;

		/* Set the 6 bit for CGRAM command */
		SET_BIT(Local_u8CGRAMAddress, 6);

		/* Send the CGRAM address */
		CLCD_voidSendCmd(Local_u8CGRAMAddress);

		/* Write the input pattern inside the CGRAM */
		for(Local_u8Counter = 0u; Local_u8Counter < 8; Local_u8Counter++)
		{
			CLCD_voidSendData(Copy_pu8Pattern[Local_u8Counter]);
		}

		/* Go back to DDRAM */
		CLCD_voidGoToXY(Copy_u8XPos, Copy_u8YPos);

		/* Display the input pattern inside CGRAM */
		CLCD_voidSendData(Copy_LocationNum);
	}
	else
	{
		Local_u8ErrorState = NULL_PTR_ERR;
	}

	return Local_u8ErrorState;
}




