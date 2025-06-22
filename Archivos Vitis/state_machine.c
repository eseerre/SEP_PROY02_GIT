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
#include "I2C.h"
#include "xscugic.h"

#include "IMG_LCD.h"


#define INT_ID_BTN_Z0		61 //[7:0] --> [68:61]
#define INT_ID_BTN_Z1		62
#define INT_ID_BTN_Z2		63
#define INT_ID_BTN_Z3		64
#define INT_ID_BTN_B0		65
#define INT_ID_BTN_B1		66
#define INT_ID_BTN_BJS		67
#define INT_ID_TMR			68




int start_up();

int refresh();

int menu_inicio();

int test_rapido();

extern XGpio GPIO_LCD;
extern XGpio GPIO_SW;
extern XSpi  SPI_LCD;	 /* The instance of the SPI device */
extern XSpi  SPI_ADC;

extern XScuGic_Config *GIC_CONFIG;
extern XScuGic GIC;

extern int booster_0;
extern int booster_1;
extern int timeout_menu_inicial;

extern char joyx[16] = {};
extern char joyy[16] = {};
extern int joyx_min = 512;
extern int joyx_max = 512;
extern char jx_min[16] = {};
extern char jx_max[16] = {};
extern int joyy_min = 512;
extern int joyy_max = 512;
extern char jy_min[16] = {};
extern char jy_max[16] = {};


extern char acx[16] = {};
extern char acy[16] = {};
extern char acz[16] = {};

extern char temp[16] = {};
extern char luz[16] = {};

extern char pot1[16] = {};
extern char pot2[16] = {};
extern int pot1_min = 512;
extern int pot1_max = 512;
extern char p1_min[16] = {};
extern char p1_max[16] = {};
extern int pot2_min = 512;
extern int pot2_max = 512;
extern char p2_min[16] = {};
extern char p2_max[16] = {};

extern char mic[16] = {};
extern char temp[16] = {};
extern char luz[16] = {};

int main(){
    int sel_inicio;

    start_up();
	sel_inicio = menu_inicio();
	if (sel_inicio == -1){
		xil_printf("Proceso no fue comenzado");
		GUI_DisString_EN(42, 64, "Adios", &Font12, GUI_BACKGROUND, CYAN);
		delay_ms(5000);
		LCD_Clear(GUI_BACKGROUND);
		return -1;
	}
	else if (sel_inicio == 0){
		LCD_Clear(GUI_BACKGROUND);
		test_botones();
	}
	else{
		LCD_Clear(GUI_BACKGROUND);
		test_rapido();
	}


}


int start_up(){

    int Status;

    init_platform();

    GIC_CONFIG = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
    if (NULL == GIC_CONFIG){
    	return XST_FAILURE;
    }

    Status = XScuGic_CfgInitialize(&GIC, GIC_CONFIG, GIC_CONFIG->CpuBaseAddress);
    if (Status != XST_SUCCESS){
    	xil_printf("GIC Initialize failed\r\n");
    	return XST_FAILURE;
    }

    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler, $GIC);
    Xil_ExceptionEnable();

    Status = XScuGic_Connect(&GIC, INT_ID_BTN_B0, (Xil_InterruptHandler) btn_b0_inicio, NULL);
	if (Status != XST_SUCCESS) {
		xil_printf("Interrupt B_B0 Failed\r\n");
		return XST_FAILURE;
	}
	Status = XScuGic_Connect(&GIC, INT_ID_BTN_B1, (Xil_InterruptHandler) btn_b1_inicio, NULL);
		if (Status != XST_SUCCESS) {
			xil_printf("Interrupt B_B1 Failed\r\n");
			return XST_FAILURE;
		}

    Status = init_IIC();
	if (Status != XST_SUCCESS) {
		xil_printf("IIC Mode Failed\r\n");
		return XST_FAILURE;
	}

    Status = XGpio_Initialize(&GPIO_LCD, XPAR_AXI_GPIO_0_DEVICE_ID); // Revisar ID
	if (Status != XST_SUCCESS) {
		xil_printf("GPIO-LCD Initialization Failed\r\n");
		return XST_FAILURE;
	}

    Status = XGpio_Initialize(&GPIO_SW, XPAR_AXI_GPIO_1_DEVICE_ID); // Revisar ID
	if (Status != XST_SUCCESS) {
		xil_printf("GPIO-SW Initialization Failed\r\n");
		return XST_FAILURE;
	}

	// Set up the AXI SPI Controller
	Status = XSpi_Init(&SPI_LCD, SPI_DEVICE_ID); // Revisar ID
	if (Status != XST_SUCCESS) {
		xil_printf("SPI-LCD Mode Failed\r\n");
		return XST_FAILURE;
	}

	Status = init_adc(&SPI_ADC, SPI_DEVICE_ID_1); // Revisar ID
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

    return 0;
}


int menu_inicio(){
    
	//Configurar interrupt del timer y de los botones


	GUI_IMG(booster);

    GUI_DisString_EN(15,8, "Hola! Presione",&Font12, GUI_BACKGROUND,CYAN);
    GUI_DisString_EN(22,24, "el boton del",&Font12, GUI_BACKGROUND,CYAN);
    GUI_DisString_EN(15,40, "modo de testeo",&Font12, GUI_BACKGROUND,CYAN);
	GUI_DisString_EN(65, 71, "-> Paso a", &Font12, GUI_BACKGROUND, CYAN);
	GUI_DisString_EN(99, 79, "paso", &Font12, GUI_BACKGROUND, CYAN);
	GUI_DisString_EN(65, 92, "-> Modo", &Font12, GUI_BACKGROUND, CYAN);
	GUI_DisString_EN(72, 71, "rapido", &Font12, GUI_BACKGROUND, CYAN);

	// Esperar el interrupt de los botones, retornar dependiendo del que se recibe.
	while(1){
		if (booster_0 == 1){
			LCD_Clear(GUI_BACKGROUND);
			return 0;
		}
		else if(booster_1 == 1){
			LCD_Clear(GUI_BACKGROUND);
			return 1;
		}
		else if(timeout_menu_inicial == 1){
			LCD_Clear(GUI_BACKGROUND);
			return -1;
		}
	}
}

int paso_a_paso(){

}

int test_rapido(){
	GUI_DisString_EN(15,8, "Presione los", &Font12, GUI_BACKGROUND,CYAN);
	GUI_DisString_EN(22,24, "botones", &Font12, GUI_BACKGROUND,CYAN);
	GUI_DisString_EN(15,40, "solo una vez", &Font12, GUI_BACKGROUND,CYAN);

	Xil_Out32(XPAR_AXI_CONNECTIONS_0_S00_AXI_BASEADDR + 8, 0xFFFFFFFF); //Encender Buzzer
	LCD_Clear(WHITE);
	delay_ms(3000);
	LCD_Clear(GUI_BACKGROUND);
	Xil_Out32(XPAR_AXI_CONNECTIONS_0_S00_AXI_BASEADDR + 8, 0x0); //Apagar Buzzer

	// Mostrar todo:
	GUI_DisString_EN(0, 0, "Zybo Z7-10", &Font12, GUI_BACKGROUND, WHITE); //Seccion Zybo

	GUI_DisString_EN(2, 14, "BTN0", &Font8, GUI_BACKGROUND, RED); //BTN0
	GUI_DisString_EN(27, 14, "BTN1", &Font8, GUI_BACKGROUND, RED); //BTN1
	GUI_DisString_EN(52, 14, "BTN2", &Font8, GUI_BACKGROUND, RED); //BTN2
	GUI_DisString_EN(77, 14, "BTN3", &Font8, GUI_BACKGROUND, RED); //BTN3

	GUI_DisString_EN(104, 0, "SW0", &Font8, GUI_BACKGROUND, WHITE); //SW0
	GUI_DisString_EN(104, 8, "SW1", &Font8, GUI_BACKGROUND, WHITE); //SW1
	GUI_DisString_EN(104, 16, "SW2", &Font8, GUI_BACKGROUND, WHITE); //SW2
	GUI_DisString_EN(104, 24, "SW3", &Font8, GUI_BACKGROUND, WHITE); //SW3

	GUI_DisString_EN(0, 24, "BOOSTER PACK", &Font12, GUI_BACKGROUND, WHITE); //Seccion Booster

	GUI_DisString_EN(0, 38, "EjeX", &Font8, GUI_BACKGROUND, WHITE); //Acelerometro X
	GUI_DisString_EN(0, 47, "EjeY", &Font8, GUI_BACKGROUND, WHITE); //Acelerometro Y
	GUI_DisString_EN(0, 56, "EjeZ", &Font8, GUI_BACKGROUND, WHITE); //Acelerometro Z

	GUI_DisString_EN(48, 40, "T", &Font8, GUI_BACKGROUND, WHITE); //Temperatura
	GUI_DisString_EN(48, 48, "Mic", &Font8, GUI_BACKGROUND, WHITE); //Microfono
	GUI_DisString_EN(48, 56, "Luz", &Font8, GUI_BACKGROUND, WHITE); //Luz

	GUI_DisString_EN(106, 40, "BTN0", &Font8, GUI_BACKGROUND, RED); //BTN0
	GUI_DisString_EN(106, 48, "BTN1", &Font8, GUI_BACKGROUND, RED); //BTN1
	GUI_DisString_EN(106, 56, "B JS", &Font8, GUI_BACKGROUND, RED); //BTN JS

	GUI_DisString_EN(0, 64, "Joystick", &Font12, GUI_BACKGROUND, WHITE); //Joystick

	GUI_DisString_EN(0, 80, "X actual", &Font8, GUI_BACKGROUND, WHITE); //Valores Joystick
	GUI_DisString_EN(0, 88, "Y actual", &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(0, 96, "X minimo", &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(0, 104, "X maximo", &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(0, 112, "Y minimo", &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(0, 120, "Y maximo", &Font8, GUI_BACKGROUND, WHITE);

	GUI_DisString_EN(64, 68, "Potenciometro", &Font8, GUI_BACKGROUND, WHITE); //Potenciometro

	GUI_DisString_EN(64, 80, "I actual", &Font8, GUI_BACKGROUND, WHITE); //Valores Potenciometro
	GUI_DisString_EN(64, 88, "D actual", &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(64, 96, "I minimo", &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(64, 104, "I maximo", &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(64, 112, "D minimo", &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(64, 120, "D maximo", &Font8, GUI_BACKGROUND, WHITE);

	Xil_Out32(XPAR_AXI_CONNECTIONS_0_S00_AXI_BASEADDR, 0xFFFFFFFF); //Encender LED RGB
	Xil_Out32(XPAR_AXI_CONNECTIONS_0_S00_AXI_BASEADDR + 4, 0xFFFFFFFF); // Encender LEDs

	XScuGic_Connect($GIC, INT_ID_BTN_Z0, (Xil_InterruptHandler) btn_z0_fast_mode, NULL);
	XScuGic_Connect($GIC, INT_ID_BTN_Z1, (Xil_InterruptHandler) btn_z1_fast_mode, NULL);
	XScuGic_Connect($GIC, INT_ID_BTN_Z2, (Xil_InterruptHandler) btn_z2_fast_mode, NULL);
	XScuGic_Connect($GIC, INT_ID_BTN_Z3, (Xil_InterruptHandler) btn_z3_fast_mode, NULL);
	XScuGic_Connect($GIC, INT_ID_BTN_B0, (Xil_InterruptHandler) btn_b0_fast_mode, NULL);
	XScuGic_Connect($GIC, INT_ID_BTN_B1, (Xil_InterruptHandler) btn_b1_fast_mode, NULL);
	XScuGic_Connect($GIC, INT_ID_BTN_BJS, (Xil_InterruptHandler) btn_bjs_fast_mode, NULL);

	XScuGic_Connect($GIC, INT_ID_TMR, (Xil_Interrupt_Handler) quick_peripherals, NULL); // Ver bien como se manejan los interrupts del axi timer

	refresh();



}

int refresh(){
	int sw;
	int sw0;
	int sw1;
	int sw2;
	int sw3;

	
	while(1){

		GUI_DisString_EN(120, 0, sw0, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
		GUI_DisString_EN(120, 8, sw1, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
		GUI_DisString_EN(120, 16, sw2, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
		GUI_DisString_EN(120, 24, sw3, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);

		sw = XGpio_DiscreteRead(&GPIO_SW, 1);
		sw0 = sw % 2;
		sw1 = ((sw - sw0) / 2) % 2;
		sw2 = (((sw - sw0) / 2 - sw1) / 2) % 2;
		sw3 = (((((sw - sw0) / 2 - sw1) / 2) - sw2) / 2) % 2;

		GUI_DisString_EN(120, 0, sw0, &Font8, GUI_BACKGROUND, WHITE);
		GUI_DisString_EN(120, 8, sw1, &Font8, GUI_BACKGROUND, WHITE);
		GUI_DisString_EN(120, 16, sw2, &Font8, GUI_BACKGROUND, WHITE);
		GUI_DisString_EN(120, 24, sw3, &Font8, GUI_BACKGROUND, WHITE);

		delay_ms(50);

	}
	return 0;
}
