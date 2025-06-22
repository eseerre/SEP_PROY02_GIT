#include <stdio.h>
#include <sleep.h>
#include <time.h>
#include <unistd.h>

#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xstatus.h"
#include "Delay.h"
#include "LCD_SPI.h"
#include "LCD_Driver.h"
#include "LCD_GUI.h"
#include "ADC.h"



extern XGpio gpio0;
extern XSpi  SpiInstance;	 /* The instance of the SPI device */
extern XSpi  SpiInstance1;
extern const unsigned char font[] ;

#define BACKGROUND  WHITE
#define FOREGROUND BLUE
#define DELAY 1000




int main()
{
	int Status;

    //Initialize the UART
    init_platform();
	/* Initialize the GPIO 0 driver */
	Status = XGpio_Initialize(&gpio0, XPAR_AXI_GPIO_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Gpio 0 Initialization Failed\r\n");
		return XST_FAILURE;
	}

	// Set up the AXI SPI Controller
	Status = XSpi_Init(&SpiInstance,SPI_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("SPI Mode Failed\r\n");
		return XST_FAILURE;
	}

	Status = init_adc(&SpiInstance1, SPI_DEVICE_ID_1);
	if (Status != XST_SUCCESS) {
		xil_printf("SPI-ADC Mode Failed\r\n");
		return XST_FAILURE;
	}
	xil_printf("TFT initialized \r\n");

	xil_printf("**********Init LCD**********\r\n");
	LCD_SCAN_DIR LCD_ScanDir = SCAN_DIR_DFT;//SCAN_DIR_DFT = D2U_L2R
	LCD_Init(LCD_ScanDir );

	xil_printf("LCD Show \r\n");
	//GUI_Show();
	//delay_ms(1000);
	LCD_Clear(GUI_BACKGROUND);
	GUI_INTRO();
	delay_ms(5000);
	LCD_Clear(GUI_BACKGROUND);
	char joyx[16] = {};
	char joyy[16] = {};
	char acx[16] = {};
	char acy[16] = {};
	char acz[16] = {};
	char pot1[16] = {};
	char pot2[16] = {};
    while(1){


		GUI_DisString_EN(5,10,"Ejex",&Font16,GUI_BACKGROUND,CYAN);
		GUI_DisString_EN(5,70,"EjeY",&Font16,GUI_BACKGROUND,CYAN);
		GUI_DisString_EN(70,10,"Pot1",&Font16,GUI_BACKGROUND,CYAN);
		GUI_DisString_EN(70,70,"Pot2",&Font16,GUI_BACKGROUND,CYAN);


		GUI_DisString_EN(5,40,joyx,&Font16,GUI_BACKGROUND,GUI_BACKGROUND);
		GUI_DisString_EN(5,100,joyy,&Font16,GUI_BACKGROUND,GUI_BACKGROUND);
		GUI_DisString_EN(70,40,pot1,&Font16,GUI_BACKGROUND,GUI_BACKGROUND);
		GUI_DisString_EN(70,100,pot2,&Font16,GUI_BACKGROUND,GUI_BACKGROUND);

		xil_printf("JX :%d\n", read_joyx());
		xil_printf("JY :%d\n", read_joyy());
		xil_printf("ACX :%d\n", read_acx());
		xil_printf("ACY :%d\n", read_acy());
		xil_printf("ACZ :%d\n", read_acz());
		xil_printf("MIC :%d\n", read_MIC());
		xil_printf("POT1 :%d\n", read_POT1());
		xil_printf("POT2 :%d\n", read_POT2());
		xil_printf("\n");


		sprintf(joyx, "%d", read_joyx());
		sprintf(joyy, "%d", read_joyy());
		sprintf(acx, "%d", read_acx());
		sprintf(acy, "%d", read_acy());
		sprintf(acz, "%d", read_acz());
		sprintf(pot1, "%d", read_POT1());
		sprintf(pot2, "%d", read_POT2());

		GUI_DisString_EN(5,40,joyx,&Font16,GUI_BACKGROUND,YELLOW);
		GUI_DisString_EN(5,100,joyy,&Font16,GUI_BACKGROUND,YELLOW);
		GUI_DisString_EN(70,40,pot1,&Font16,GUI_BACKGROUND,YELLOW);
		GUI_DisString_EN(70,100,pot2,&Font16,GUI_BACKGROUND,YELLOW);

		delay_ms(50);
    }
    return 0;

}
