/*
 * functies.c
 *
 *  Created on: 6-jun.-2017
 *      Author: paull
 */

#include "functies.h"
#include "startscherm.h"



void initLCD( void )
{
  /* LCD Initialization */
  BSP_LCD_Init();
  BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
  /* Enable the LCD */
  BSP_LCD_DisplayOn();
  /* Select the LCD Background Layer  */
  BSP_LCD_SelectLayer(0);
  /* Clear the Background Layer */
  BSP_LCD_Clear(LCD_COLOR_RED);
  BSP_LCD_SetBackColor(LCD_COLOR_RED);
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_TS_Init(480,272);
  WDA_LCD_DrawBitmap(SLIMSTEMENS_DATA,0,0,SLIMSTEMENS_DATA_X_PIXEL,SLIMSTEMENS_DATA_Y_PIXEL,SLIMSTEMENS_DATA_FORMAT);

  BSP_LCD_SetFont(&Font20);


}

void background( void )
{
	  WDA_LCD_DrawBitmap(SLIMSTEMENS_DATA,0,0,SLIMSTEMENS_DATA_X_PIXEL,SLIMSTEMENS_DATA_Y_PIXEL,SLIMSTEMENS_DATA_FORMAT);

}


err_t imageOntvangen(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{



		static int flag = 0;
		static unsigned short data[6000];
		static uint16_t receivedLen = 0;
		struct pbuf * buffer = p;
		static int teller = 0;
		static int juist;
		int i;



		//char test2 = ((char*)buffer->payload)[0];
		//char test3 = ((char*)buffer->payload)[1];
		uint16_t lengte = buffer->len;

		for(i = 0; i<lengte-1;i += 2)
		{
			char get1 = ((char*)buffer->payload)[i];
			char get2 = ((char*)buffer->payload)[i+1];

			data[teller] = (get1 << 8)|get2;

			teller++;

		}


		juist = data[teller - 1];





		receivedLen += lengte;
		tcp_recved(tpcb,p->tot_len);





	//imageReceived = 1;

	char toPrint[50];
		sprintf(toPrint,"Len: %d",receivedLen);
		BSP_LCD_SetFont(&Font12);
		BSP_LCD_DisplayStringAtLine(4,toPrint);
		if(p->flags != 0)
		{
		//	tcp_recved(tpcb,receivedLen);
			char toPrint2[50] = "finito";
			BSP_LCD_SetFont(&Font12);
			BSP_LCD_DisplayStringAtLine(5,toPrint2);
			receivedLen = 0;
			teller = 0;
			pbuf_free(p);
			if(flag == 0)
			{
				WDA_LCD_DrawBitmap(data,x_offset1,y_offset,img_size,img_size,LTDC_PIXEL_FORMAT_RGB565);
				flag +=1;
			}
			else if(flag == 1)
			{
				WDA_LCD_DrawBitmap(data,x_offset2,y_offset,img_size,img_size,LTDC_PIXEL_FORMAT_RGB565);
				flag +=1;
			}
			else if(flag == 2)
			{
				WDA_LCD_DrawBitmap(data,x_offset3,y_offset,img_size,img_size,LTDC_PIXEL_FORMAT_RGB565);
				flag = 0;
			}
			return ERR_OK;
		}



		pbuf_free(p);
		return ERR_OK;



}

err_t questionOntvangen(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
		char vraag[50] = "";
		strncpy(vraag,p->payload,p->tot_len);
		BSP_LCD_SetFont(&Font16);
		BSP_LCD_DisplayStringAt(0,10,(uint8_t*)vraag,CENTER_MODE);


		tcp_recved(tpcb,p->tot_len);
//		tcp_close(tpcb);
		pbuf_free(p);
		return ERR_OK;
}


void questionrequest(struct tcp_pcb *connectie,char datam[20])
{
	uint8_t i;
	char data[20];
	strcpy(data,datam);
	connectie->flags = 0;
	tcp_write(connectie,&data,20,1);		// We sturen een q om een quote terug te krijgen
	tcp_recv(connectie,questionOntvangen);	// We willen nu wat data ontvangen
	while(!connectie->flags)
	{
		MX_LWIP_Process();
	}

	for(i=0;i<3;i++)
	{
    connectie->flags=0;
	char data2[20] = "foto0/0";
	tcp_write(connectie,&data2,sizeof(data2),1);		// We sturen een q om een quote terug te krijgen
	tcp_recv(connectie,imageOntvangen);	// We willen nu wat data ontvangen
	while(!connectie->flags)
	{
		MX_LWIP_Process();
	}
	}
}

uint8_t kies(uint16_t xpos,uint16_t ypos)
{
	  if(ypos > y_offset && ypos < (y_offset + img_size)) 			//juiste Y as
	  {
		  if(xpos > x_offset1 && xpos < (x_offset1 + img_size))		//antwoord 1
		  {
			  return 1;
		  }
		  else if(xpos > x_offset2 && xpos < (x_offset2 + img_size))	//antwoord 2
		  {
			  return 2;
		  }
		  else if(xpos > x_offset2 && xpos < (x_offset3 + img_size))	//antwoord 3
		  {
			  return 3;
		  }
		  else
		  {
			  return 0 ;
		  }

	  }
}
