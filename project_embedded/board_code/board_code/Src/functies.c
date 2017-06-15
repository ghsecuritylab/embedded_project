/*
 * functies.c
 *
 *  Created on: 6-jun.-2017
 *      Author: paull
 */

#include "functies.h"
#include "startscherm.h"

static err_t prvFunctiesImageOntvangen( void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err );

static err_t prvFunctiesQuestionOntvangen( void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err );

static uint8_t ucJuist = 0;

/*------------------------------------------------------------------------------------------------*/

void vFunctiesInitLCD( void )
{
	/* LCD Initialization */
	BSP_LCD_Init();
	BSP_LCD_LayerDefaultInit( 0, LCD_FB_START_ADDRESS );
	/* Enable the LCD */
	BSP_LCD_DisplayOn();
	/* Select the LCD vFunctiesBackground Layer  */
	BSP_LCD_SelectLayer( 0 );
	/* Clear the vFunctiesBackground Layer */
	BSP_LCD_Clear( LCD_COLOR_RED );
	BSP_LCD_SetBackColor( LCD_COLOR_RED );
	BSP_LCD_SetTextColor( LCD_COLOR_WHITE );
	BSP_TS_Init( 480, 272 );
	WDA_LCD_DrawBitmap( SLIMSTEMENS_DATA, 0, 0, SLIMSTEMENS_DATA_X_PIXEL, SLIMSTEMENS_DATA_Y_PIXEL, SLIMSTEMENS_DATA_FORMAT );
	BSP_LCD_SetFont( &Font20 );
	return;
}
/*------------------------------------------------------------------------------------------------*/

void vFunctiesBackground( void )
{
	WDA_LCD_DrawBitmap( SLIMSTEMENS_DATA, 0, 0, SLIMSTEMENS_DATA_X_PIXEL, SLIMSTEMENS_DATA_Y_PIXEL, SLIMSTEMENS_DATA_FORMAT );
	return;
}
/*------------------------------------------------------------------------------------------------*/

static err_t prvFunctiesImageOntvangen( void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err )
{

static uint8_t ucFlag = 0;
static uint16_t usData[6000] = {0};
static uint16_t usReceivedLen = 0;
struct pbuf * pxBuffer = p;
static uint16_t usTeller = 0;
static uint32_t ulJuistFoto = 0;
uint16_t usForTeller = 0;
char cGet1;
char cGet2;
char cToPrint[50];
char cToPrint2[50] = "finito";

	uint16_t usLengte = pxBuffer->len;

	for( usForTeller = 0; usForTeller<usLengte-1; usForTeller += 2 )
	{
		cGet1 = ( ( char* )pxBuffer->payload )[usForTeller];
		cGet2 = ( ( char* )pxBuffer->payload )[usForTeller+1];

		usData[usTeller] = ( cGet1 << 8 )| cGet2;
		usTeller++;
	}

	ulJuistFoto = usData[usTeller - 1];
	usReceivedLen += usLengte;
	tcp_recved( tpcb, p->tot_len );

	sprintf( cToPrint, "Len: %d", usReceivedLen );
	BSP_LCD_SetFont( &Font12 );
	BSP_LCD_DisplayStringAtLine( 20, cToPrint );

	if( p->flags != 0 )
	{

		BSP_LCD_SetFont( &Font12 );
		BSP_LCD_DisplayStringAtLine( 21, cToPrint2 );
		usReceivedLen = 0;
		usTeller = 0;
		pbuf_free( p );

		if(ucFlag == 0)
		{
			WDA_LCD_DrawBitmap(usData, XOffset1, YOffset, ImgSize, ImgSize, LTDC_PIXEL_FORMAT_RGB565 );
			ucFlag += 1;
			if( ulJuistFoto == 0x2040 )
			{
				ucJuist = 1;
			}
		}

		else if( ucFlag == 1 )
		{
			WDA_LCD_DrawBitmap( usData, XOffset2, YOffset, ImgSize, ImgSize, LTDC_PIXEL_FORMAT_RGB565 );
			ucFlag += 1;

			if( ulJuistFoto == 0x2040 )
			{
				ucJuist = 2;
			}
		}

		else if( ucFlag == 2 )
		{
			WDA_LCD_DrawBitmap( usData, XOffset3, YOffset, ImgSize,ImgSize, LTDC_PIXEL_FORMAT_RGB565 );
			ucFlag = 0;

			if( ulJuistFoto == 0x2040 )
			{
				ucJuist = 3;
			}
		}
		else
		{

		}
		return ERR_OK;
	}



	pbuf_free( p );
	return ERR_OK;



}
/*------------------------------------------------------------------------------------------------*/

static err_t prvFunctiesQuestionOntvangen( void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err )
{
char cVraag[50] = "";
	strncpy( cVraag, p->payload, p->tot_len );
	if( strcmp( cVraag, "uit" ) == 0 )
	{
		vFunctiesBackground();
		ucJuist = 4;
	}
	else
	{
		BSP_LCD_SetFont(&Font16);
		BSP_LCD_DisplayStringAt( 0, 50, ( uint8_t* )cVraag, CENTER_MODE );
	}


	tcp_recved( tpcb, p->tot_len );
	pbuf_free( p );
	return ERR_OK;
}
/*------------------------------------------------------------------------------------------------*/

 uint8_t ucFunctiesQuestionRequest( struct tcp_pcb *connectie, char cVraag[2] )
{
char cDataSend[20] = "vraag/";
char cDataSend2[20] = "foto0/";

	connectie->flags = 0;

	strcat( cDataSend, cVraag );
	tcp_write( connectie, cDataSend, sizeof( cDataSend ), 1 );		// We sturen een q om een quote terug te krijgen
	tcp_recv( connectie, prvFunctiesQuestionOntvangen );	// We willen nu wat data ontvangen
	while( !connectie->flags )
	{
		MX_LWIP_Process();
	}

	if( ucJuist != 4 )
	{
		strcat( cDataSend2, cVraag );
		connectie->flags=0;
		tcp_write( connectie, &cDataSend2, sizeof( cDataSend2 ), 1 );		// We sturen een q om een quote terug te krijgen
		tcp_recv( connectie, prvFunctiesImageOntvangen );	// We willen nu wat data ontvangen

		while( !connectie->flags )
		{
			MX_LWIP_Process();
		}

		connectie->flags=0;
		cDataSend2[4] = '1';
		tcp_write( connectie, &cDataSend2, sizeof( cDataSend2 ),1 );		// We sturen een q om een quote terug te krijgen
		tcp_recv( connectie, prvFunctiesImageOntvangen );	// We willen nu wat data ontvangen

		while( !connectie->flags )
		{
			MX_LWIP_Process();
		}

		connectie->flags=0;
		cDataSend2[4] = '2';
		tcp_write( connectie,&cDataSend2, sizeof( cDataSend2 ),1 );		// We sturen een q om een quote terug te krijgen
		tcp_recv( connectie, prvFunctiesImageOntvangen );	// We willen nu wat data ontvangen

		while( !connectie->flags )
		{
			MX_LWIP_Process();
		}
	}

	return ucJuist;
}
/*------------------------------------------------------------------------------------------------*/

uint8_t ucFunctiesKies( uint16_t xpos, uint16_t ypos )
{
	if( ( ypos > YOffset ) && ( ypos < ( YOffset + ImgSize ) ) ) 			//juiste Y as
	{
		  if( ( xpos > XOffset1 ) && ( xpos < ( XOffset1 + ImgSize ) ) )		//antwoord 1
		  {
			  return 1;
		  }
		  else if( ( xpos > XOffset2 ) && ( xpos < ( XOffset2 + ImgSize ) ) )	//antwoord 2
		  {
			  return 2;
		  }
		  else if( ( xpos > XOffset2 ) && ( xpos < ( XOffset3 + ImgSize ) ) )	//antwoord 3
		  {
			  return 3;
		  }
		  else
		  {
			  return 0 ;
		  }

	}
	return 0 ;
}
/*------------------------------------------------------------------------------------------------*/
