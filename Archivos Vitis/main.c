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
#include "xtmrctr.h"


#define INT_ID_BTN_Z0		61 //[7:0] --> [68:61]
#define INT_ID_BTN_Z1		62
#define INT_ID_BTN_Z2		63
#define INT_ID_BTN_Z3		64
#define INT_ID_BTN_B0		65
#define INT_ID_BTN_B1		66
#define INT_ID_BTN_BJS		67
#define INT_ID_TMR_SLOW		XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR
#define INT_ID_TMR_FAST		XPAR_FABRIC_AXI_TIMER_1_INTERRUPT_INTR

#define RESET_VALUE_SLOW	0xFA000000
#define RESET_VALUE_FAST	0xFF2F0000



void btn_b0_inicio(void *CallbackRef);
void btn_b1_inicio(void *CallbackRef);
void tmr_inicio();

void btn_z0_fast_mode(void *CallbackRef);
void btn_z1_fast_mode(void *CallbackRef);
void btn_z2_fast_mode(void *CallbackRef);
void btn_z3_fast_mode(void *CallbackRef);

void btn_b0_fast_mode(void *CallbackRef);
void btn_b1_fast_mode(void *CallbackRef);
void btn_bjs_fast_mode(void *CallbackRef);

void quick_peripherals(void *CallbackRef);
void slow_peripherals(void *CallbackRef);

int start_up();

int refresh();

int menu_inicio();

int test_rapido();



XTmrCtr TMR_SLOW;
XTmrCtr TMR_FAST;

XGpio GPIO_SW;
extern XGpio GPIO_LCD;
extern XSpi  SPI_LCD;	 /* The instance of the SPI device */
extern XSpi  SPI_ADC;

XScuGic_Config *GIC_CONFIG;
XScuGic GIC;

int booster_0;
int booster_1;
int timeout_menu_inicial;

char joyx[16] = {};
char joyy[16] = {};

char acx[16] = {};
char acy[16] = {};
char acz[16] = {};

char pot1[16] = {};
char pot2[16] = {};

char mic[16] = {};
char temp[16] = {};
char luz[16] = {};

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
		//test_botones();
		return 0;
	}
	else{
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

    XScuGic_SetPriorityTriggerType(&GIC, INT_ID_BTN_B0, 0x18, 0x3);
    XScuGic_SetPriorityTriggerType(&GIC, INT_ID_BTN_B1, 0x10, 0x3);
    XScuGic_SetPriorityTriggerType(&GIC, INT_ID_TMR_FAST, 0x00, 0x3);
    XScuGic_SetPriorityTriggerType(&GIC, INT_ID_TMR_SLOW, 0x08, 0x3);

    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler, &GIC);
    Xil_ExceptionEnable();

    Status = XTmrCtr_Initialize(&TMR_SLOW, XPAR_TMRCTR_0_DEVICE_ID);
	if (Status != XST_SUCCESS){
		xil_printf("TMR SLOW Initialize failed\r\n");
		return XST_FAILURE;
		}
	XTmrCtr_SetOptions(&TMR_SLOW, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
	XTmrCtr_SetResetValue(&TMR_SLOW, 0, 0xC0000000);

	XScuGic_Connect(&GIC, INT_ID_TMR_SLOW, (Xil_InterruptHandler) tmr_inicio, NULL);
	XScuGic_Enable(&GIC, INT_ID_TMR_SLOW);


	Status = XTmrCtr_Initialize(&TMR_FAST, XPAR_TMRCTR_1_DEVICE_ID);
	if (Status != XST_SUCCESS){
		xil_printf("TMR FAST Initialize failed\r\n");
		return XST_FAILURE;
		}
	XTmrCtr_SetResetValue(&TMR_FAST, 0, RESET_VALUE_FAST);
	XTmrCtr_SetOptions(&TMR_FAST, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);

    Status = XScuGic_Connect(&GIC, INT_ID_BTN_B0, (Xil_InterruptHandler) btn_b0_inicio, NULL);
	if (Status != XST_SUCCESS) {
		xil_printf("Interrupt B_B0 Failed\r\n");
		return XST_FAILURE;
	}
	XScuGic_Enable(&GIC, INT_ID_BTN_B0);

	Status = XScuGic_Connect(&GIC, INT_ID_BTN_B1, (Xil_InterruptHandler) btn_b1_inicio, NULL);
		if (Status != XST_SUCCESS) {
			xil_printf("Interrupt B_B1 Failed\r\n");
			return XST_FAILURE;
		}
	XScuGic_Enable(&GIC, INT_ID_BTN_B1);

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
	Status = XSpi_Init(&SPI_LCD, XPAR_AXI_QUAD_SPI_0_DEVICE_ID); // Revisar ID
	if (Status != XST_SUCCESS) {
		xil_printf("SPI-LCD Mode Failed\r\n");
		return XST_FAILURE;
	}

	Status = XSpi_Init(&SPI_ADC,XPAR_AXI_QUAD_SPI_1_DEVICE_ID); // Revisar ID
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


	GUI_IMG();

    GUI_DisString_EN(15,8, "Hola! Presione",&Font12, GUI_BACKGROUND,CYAN);
    GUI_DisString_EN(22,24, "el boton del",&Font12, GUI_BACKGROUND,CYAN);
    GUI_DisString_EN(15,40, "modo de testeo",&Font12, GUI_BACKGROUND,CYAN);
	GUI_DisString_EN(58, 65, "-> Paso a", &Font12, GUI_BACKGROUND, CYAN);
	GUI_DisString_EN(65, 77, "paso", &Font12, GUI_BACKGROUND, CYAN);
	GUI_DisString_EN(65, 90, "-> Modo", &Font12, GUI_BACKGROUND, CYAN);
	GUI_DisString_EN(65, 102, "rapido", &Font12, GUI_BACKGROUND, CYAN);
	XTmrCtr_Start(&TMR_SLOW, 0);

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

//int paso_a_paso(){}

int test_rapido(){

	int status = 1;

	GUI_DisString_EN(15,8, "Presione los", &Font12, GUI_BACKGROUND,CYAN);
	GUI_DisString_EN(22,24, "botones", &Font12, GUI_BACKGROUND,CYAN);
	GUI_DisString_EN(15,40, "solo una vez", &Font12, GUI_BACKGROUND,CYAN);
	delay_ms(1000);
	Xil_Out32(XPAR_AXI_CONNECTIONS_0_S00_AXI_BASEADDR + 8, 0xFFFFFFFF); //Encender Buzzer
	LCD_Clear(WHITE);
	delay_ms(2300);
	LCD_Clear(GUI_BACKGROUND);
	Xil_Out32(XPAR_AXI_CONNECTIONS_0_S00_AXI_BASEADDR + 8, 0x0); //Apagar Buzzer

	// Mostrar todo:
	GUI_DisString_EN(0, 2, "Zybo Z7-10", &Font16, GUI_BACKGROUND, WHITE); //Seccion Zybo

	GUI_DisString_EN(2, 26, "BTN3", &Font12, GUI_BACKGROUND, BLUE); //BTN3
	GUI_DisString_EN(34, 26, "BTN2", &Font12, GUI_BACKGROUND, BLUE); //BTN2
	GUI_DisString_EN(66, 26, "BTN1", &Font12, GUI_BACKGROUND, BLUE); //BTN1
	GUI_DisString_EN(98, 26, "BTN0", &Font12, GUI_BACKGROUND, BLUE); //BTN0

	GUI_DisString_EN(2, 16, "SW3", &Font12, GUI_BACKGROUND, WHITE); //SW3
	GUI_DisString_EN(34, 16, "SW2", &Font12, GUI_BACKGROUND, WHITE); //SW2
	GUI_DisString_EN(66, 16, "SW1", &Font12, GUI_BACKGROUND, WHITE); //SW1
	GUI_DisString_EN(98, 16, "SW0", &Font12, GUI_BACKGROUND, WHITE); //SW0

	GUI_DisString_EN(24, 37, "BOOSTER", &Font16, GUI_BACKGROUND, WHITE); //Seccion Booster
	GUI_DisString_EN(40, 52, "PACK", &Font16, GUI_BACKGROUND, WHITE); //Seccion Booster

	GUI_DisString_EN(1, 65, "X", &Font8, GUI_BACKGROUND, WHITE); //Acelerometro X
	GUI_DisString_EN(1, 74, "Y", &Font8, GUI_BACKGROUND, WHITE); //Acelerometro Y
	GUI_DisString_EN(1, 83, "Z", &Font8, GUI_BACKGROUND, WHITE); //Acelerometro Z

	GUI_DisString_EN(48, 65, "T", &Font8, GUI_BACKGROUND, WHITE); //Temperatura
	GUI_DisString_EN(48, 74, "Mic", &Font8, GUI_BACKGROUND, WHITE); //Microfono
	GUI_DisString_EN(48, 83, "Luz", &Font8, GUI_BACKGROUND, WHITE); //Luz

	GUI_DisString_EN(106, 65, "BTN0", &Font8, GUI_BACKGROUND, BLUE); //BTN0
	GUI_DisString_EN(106, 74, "BTN1", &Font8, GUI_BACKGROUND, BLUE); //BTN1
	GUI_DisString_EN(106, 83, "B JS", &Font8, GUI_BACKGROUND, BLUE); //BTN JS

	GUI_DisString_EN(1, 90, "Joystick", &Font12, GUI_BACKGROUND, WHITE); //Joystick

	GUI_DisString_EN(1, 105, "X", &Font12, GUI_BACKGROUND, WHITE); //Valores Joystick
	GUI_DisString_EN(1, 120, "Y", &Font12, GUI_BACKGROUND, WHITE);


	GUI_DisString_EN(82, 90, "POTs", &Font12, GUI_BACKGROUND, WHITE); //Potenciometro

	GUI_DisString_EN(70, 105, "I", &Font12, GUI_BACKGROUND, WHITE); //Valores Potenciometro
	GUI_DisString_EN(70, 120, "D", &Font12, GUI_BACKGROUND, WHITE);

	Xil_Out32(XPAR_AXI_CONNECTIONS_0_S00_AXI_BASEADDR, 0xFFFFFFFF); //Encender LED RGB
	Xil_Out32(XPAR_AXI_CONNECTIONS_0_S00_AXI_BASEADDR + 4, 0xFFFFFFFF); // Encender LEDs

	while (status != XST_SUCCESS){
		status = XScuGic_Connect(&GIC, INT_ID_TMR_FAST, (Xil_InterruptHandler) quick_peripherals, NULL);
	}
	XScuGic_Enable(&GIC, INT_ID_TMR_FAST);
	status = 1;
	while (status != XST_SUCCESS){
		status = XScuGic_Connect(&GIC, INT_ID_TMR_SLOW, (Xil_InterruptHandler) slow_peripherals, NULL);
		}
	XScuGic_Enable(&GIC, INT_ID_TMR_SLOW);
	status = 1;
	while (status != XST_SUCCESS){
		status = XScuGic_Connect(&GIC, INT_ID_BTN_Z0, (Xil_InterruptHandler) btn_z0_fast_mode, NULL);
		}
	XScuGic_Enable(&GIC, INT_ID_BTN_Z0);
	status = 1;
	while (status != XST_SUCCESS){
		status = XScuGic_Connect(&GIC, INT_ID_BTN_Z1, (Xil_InterruptHandler) btn_z1_fast_mode, NULL);
		}
	XScuGic_Enable(&GIC, INT_ID_BTN_Z1);
	status = 1;
	while (status != XST_SUCCESS){
		status = XScuGic_Connect(&GIC, INT_ID_BTN_Z2, (Xil_InterruptHandler) btn_z2_fast_mode, NULL);
		}
	XScuGic_Enable(&GIC, INT_ID_BTN_Z2);
	status = 1;
	while (status != XST_SUCCESS){
		status = XScuGic_Connect(&GIC, INT_ID_BTN_Z3, (Xil_InterruptHandler) btn_z3_fast_mode, NULL);
		}
	XScuGic_Enable(&GIC, INT_ID_BTN_Z3);
	status = 1;
	while (status != XST_SUCCESS){
		status = XScuGic_Connect(&GIC, INT_ID_BTN_B0, (Xil_InterruptHandler) btn_b0_fast_mode, NULL);
		}
	XScuGic_Enable(&GIC, INT_ID_BTN_B0);
	status = 1;
	while (status != XST_SUCCESS){
		status = XScuGic_Connect(&GIC, INT_ID_BTN_B1, (Xil_InterruptHandler) btn_b1_fast_mode, NULL);
		}
	XScuGic_Enable(&GIC, INT_ID_BTN_B1);
	status = 1;
	while (status != XST_SUCCESS){
		status = XScuGic_Connect(&GIC, INT_ID_BTN_BJS, (Xil_InterruptHandler) btn_bjs_fast_mode, NULL);
		}
	XScuGic_Enable(&GIC, INT_ID_BTN_BJS);

	XTmrCtr_Start(&TMR_FAST, 0);
	XTmrCtr_Start(&TMR_SLOW, 0);
	refresh();
	return 0;
}

int refresh(){
	int sw;
	int sw0;
	int sw1;
	int sw2;
	int sw3;
	char sw0_[16];
	char sw1_[16];
	char sw2_[16];
	char sw3_[16];


	while(1){
		sw = XGpio_DiscreteRead(&GPIO_SW, 1);
		sw0 = sw % 2;
		sw1 = ((sw - sw0) / 2) % 2;
		sw2 = (((sw - sw0) / 2 - sw1) / 2) % 2;
		sw3 = (((((sw - sw0) / 2 - sw1) / 2) - sw2) / 2) % 2;

		GUI_DisString_EN(25, 16, sw3_, &Font12, GUI_BACKGROUND, GUI_BACKGROUND);
		GUI_DisString_EN(57, 16, sw2_, &Font12, GUI_BACKGROUND, GUI_BACKGROUND);
		GUI_DisString_EN(89, 16, sw1_, &Font12, GUI_BACKGROUND, GUI_BACKGROUND);
		GUI_DisString_EN(120, 16, sw0_, &Font12, GUI_BACKGROUND, GUI_BACKGROUND);

		sprintf(sw0_, "%d", sw0);
		sprintf(sw1_, "%d", sw1);
		sprintf(sw2_, "%d", sw2);
		sprintf(sw3_, "%d", sw3);


		GUI_DisString_EN(25, 16, sw3_, &Font12, GUI_BACKGROUND, WHITE);
		GUI_DisString_EN(57, 16, sw2_, &Font12, GUI_BACKGROUND, WHITE);
		GUI_DisString_EN(89, 16, sw1_, &Font12, GUI_BACKGROUND, WHITE);
		GUI_DisString_EN(120, 16, sw0_, &Font12, GUI_BACKGROUND, WHITE);


		GUI_DisString_EN(2, 16, "SW3", &Font12, GUI_BACKGROUND, WHITE); //SW3
		GUI_DisString_EN(34, 16, "SW2", &Font12, GUI_BACKGROUND, WHITE); //SW2
		GUI_DisString_EN(66, 16, "SW1", &Font12, GUI_BACKGROUND, WHITE); //SW1
		GUI_DisString_EN(98, 16, "SW0", &Font12, GUI_BACKGROUND, WHITE); //SW0
		delay_ms(30);

	}
	return 0;
}

void btn_b0_inicio(void *CallbackRef){
	booster_0 = 1;
	XScuGic_Disable(&GIC, INT_ID_BTN_B0);
	XScuGic_Disable(&GIC, INT_ID_BTN_B1);
	XScuGic_Disable(&GIC, INT_ID_TMR_SLOW);

	XTmrCtr_Stop(&TMR_SLOW, 0);
	XTmrCtr_SetResetValue(&TMR_SLOW, 0, RESET_VALUE_SLOW);
	XTmrCtr_Reset(&TMR_SLOW, 0);
}

void btn_b1_inicio(void *CallbackRef){
	booster_1 = 1;
	XScuGic_Disable(&GIC, INT_ID_BTN_B0);
	XScuGic_Disable(&GIC, INT_ID_BTN_B1);
	XScuGic_Disable(&GIC, INT_ID_TMR_SLOW);

	XTmrCtr_Stop(&TMR_SLOW, 0);
	XTmrCtr_SetResetValue(&TMR_SLOW, 0, RESET_VALUE_SLOW);
	XTmrCtr_Reset(&TMR_SLOW, 0);
}

void btn_z0_fast_mode(void *CallbackRef){
	GUI_DisString_EN(98, 26, "BTN0", &Font12, GUI_BACKGROUND, GREEN); //BTN0
}

void btn_z1_fast_mode(void *CallbackRef){
	GUI_DisString_EN(66, 26, "BTN1", &Font12, GUI_BACKGROUND, GREEN); //BTN1
}

void btn_z2_fast_mode(void *CallbackRef){
	GUI_DisString_EN(34, 26, "BTN2", &Font12, GUI_BACKGROUND, GREEN); //BTN2
}

void btn_z3_fast_mode(void *CallbackRef){
	GUI_DisString_EN(2, 26, "BTN3", &Font12, GUI_BACKGROUND, GREEN); //BTN3
}

void btn_b0_fast_mode(void *CallbackRef){
	GUI_DisString_EN(106, 65, "BTN0", &Font8, GUI_BACKGROUND, GREEN); //BTN0
}

void btn_b1_fast_mode(void *CallbackRef){
	GUI_DisString_EN(106, 74, "BTN1", &Font8, GUI_BACKGROUND, GREEN); //BTN1
}

void btn_bjs_fast_mode(void *CallbackRef){
	GUI_DisString_EN(106, 83, "B JS", &Font8, GUI_BACKGROUND, GREEN); //BTN JS
}

void quick_peripherals(void *CallbackRef){
	xil_printf("Interrupcion FAST\r\n");
	XTmrCtr_Stop(&TMR_FAST, 0);
	XTmrCtr_Reset(&TMR_FAST, 0);

	GUI_DisString_EN(15, 105, joyx, &Font12, GUI_BACKGROUND, GUI_BACKGROUND);
	GUI_DisString_EN(15, 120, joyy, &Font12, GUI_BACKGROUND, GUI_BACKGROUND);

	sprintf(joyx, "%d", read_joyx());
	sprintf(joyy, "%d", read_joyy());

	GUI_DisString_EN(15, 105, joyx, &Font12, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(15, 120, joyy, &Font12, GUI_BACKGROUND, WHITE);

	GUI_DisString_EN(88, 105, pot1, &Font12, GUI_BACKGROUND, GUI_BACKGROUND);
	GUI_DisString_EN(88, 120, pot2, &Font12, GUI_BACKGROUND, GUI_BACKGROUND);

	sprintf(pot1, "%d", read_POT1());
	sprintf(pot2, "%d", read_POT2());

	GUI_DisString_EN(88, 105, pot1, &Font12, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(88, 120, pot2, &Font12, GUI_BACKGROUND, WHITE);


	GUI_DisString_EN(14, 65, acx, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
	GUI_DisString_EN(14, 74, acy, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
	GUI_DisString_EN(14, 83, acz, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);

	sprintf(acx, "%d", read_acx());
	sprintf(acy, "%d", read_acy());
	sprintf(acz, "%d", read_acz());

	GUI_DisString_EN(14, 65, acx, &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(14, 74, acy, &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(14, 83, acz, &Font8, GUI_BACKGROUND, WHITE);


	GUI_DisString_EN(66, 74, mic, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);

	sprintf(mic, "%d", read_MIC());

	GUI_DisString_EN(66, 74, mic, &Font8, GUI_BACKGROUND, WHITE);

	XTmrCtr_Start(&TMR_FAST, 0);

}

void slow_peripherals(void *CallbackRef){
	XTmrCtr_Stop(&TMR_SLOW, 0);
	XTmrCtr_Reset(&TMR_SLOW, 0);

	GUI_DisString_EN(66, 65, temp, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
	GUI_DisString_EN(66, 83, luz, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
	sprintf(temp, "%d", read_tmp());
	sprintf(luz, "%d", read_opt());
	GUI_DisString_EN(66, 65, temp, &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(66, 83, luz, &Font8, GUI_BACKGROUND, WHITE);
	XTmrCtr_Start(&TMR_SLOW, 0);
}


void tmr_inicio (){
	timeout_menu_inicial = 1;;
	XScuGic_Disable(&GIC, INT_ID_TMR_SLOW);
	XScuGic_Disable(&GIC, INT_ID_BTN_B0);
	XScuGic_Disable(&GIC, INT_ID_BTN_B1);
	XTmrCtr_Stop(&TMR_SLOW, 0);
}
