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



#define INT_ID_BTN_Z0		61 //[7:0] --> [68:61]
#define INT_ID_BTN_Z1		62
#define INT_ID_BTN_Z2		63
#define INT_ID_BTN_Z3		64
#define INT_ID_BTN_B0		65
#define INT_ID_BTN_B1		66
#define INT_ID_BTN_BJS		67
#define INT_ID_TMR			68


void btn_b0_inicio(void *CallbackRef);
void btn_b1_inicio(void *CallbackRef);

void btn_z0_fast_mode(void *CallbackRef);
void btn_z1_fast_mode(void *CallbackRef);
void btn_z2_fast_mode(void *CallbackRef);
void btn_z3_fast_mode(void *CallbackRef);

void btn_b0_fast_mode(void *CallbackRef);
void btn_b1_fast_mode(void *CallbackRef);
void btn_bjs_fast_mode(void *CallbackRef);

void quick_peripherals(void *CallbackRef);

int start_up();

int refresh();

int menu_inicio();

int test_rapido();

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
int joyx_min = 512;
int joyx_max = 512;
char jx_min[16] = {};
char jx_max[16] = {};
int joyy_min = 512;
int joyy_max = 512;
char jy_min[16] = {};
char jy_max[16] = {};

char acx[16] = {};
char acy[16] = {};
char acz[16] = {};

char pot1[16] = {};
char pot2[16] = {};
int pot1_min = 512;
int pot1_max = 512;
char p1_min[16] = {};
char p1_max[16] = {};
int pot2_min = 512;
int pot2_max = 512;
char p2_min[16] = {};
char p2_max[16] = {};

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

    XScuGic_SetPriorityTriggerType(&GIC, INT_ID_BTN_B0, 0xA0, 0x3);
    XScuGic_SetPriorityTriggerType(&GIC, INT_ID_BTN_B1, 0xA0, 0x3);

    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler, &GIC);
    Xil_ExceptionEnable();

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


	GUI_IMG();

    GUI_DisString_EN(15,8, "Hola! Presione",&Font12, GUI_BACKGROUND,CYAN);
    GUI_DisString_EN(22,24, "el boton del",&Font12, GUI_BACKGROUND,CYAN);
    GUI_DisString_EN(15,40, "modo de testeo",&Font12, GUI_BACKGROUND,CYAN);
	GUI_DisString_EN(58, 65, "-> Paso a", &Font12, GUI_BACKGROUND, CYAN);
	GUI_DisString_EN(65, 77, "paso", &Font12, GUI_BACKGROUND, CYAN);
	GUI_DisString_EN(65, 90, "-> Modo", &Font12, GUI_BACKGROUND, CYAN);
	GUI_DisString_EN(65, 102, "rapido", &Font12, GUI_BACKGROUND, CYAN);

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
		/*else if(timeout_menu_inicial == 1){
			LCD_Clear(GUI_BACKGROUND);
			return -1;
		}*/
	}
}

//int paso_a_paso(){}

int test_rapido(){
	GUI_DisString_EN(15,8, "Presione los", &Font12, GUI_BACKGROUND,CYAN);
	GUI_DisString_EN(22,24, "botones", &Font12, GUI_BACKGROUND,CYAN);
	GUI_DisString_EN(15,40, "solo una vez", &Font12, GUI_BACKGROUND,CYAN);
	delay_ms(1000);
	Xil_Out32(XPAR_AXI_CONNECTIONS_0_S00_AXI_BASEADDR + 8, 0xFFFFFFFF); //Encender Buzzer
	LCD_Clear(WHITE);
	delay_ms(3000);
	LCD_Clear(GUI_BACKGROUND);
	Xil_Out32(XPAR_AXI_CONNECTIONS_0_S00_AXI_BASEADDR + 8, 0x0); //Apagar Buzzer

	// Mostrar todo:
	GUI_DisString_EN(0, 2, "Zybo Z7-10", &Font12, GUI_BACKGROUND, WHITE); //Seccion Zybo

	GUI_DisString_EN(2, 16, "BTN0", &Font8, GUI_BACKGROUND, BLUE); //BTN0
	GUI_DisString_EN(27, 16, "BTN1", &Font8, GUI_BACKGROUND, RED); //BTN1
	GUI_DisString_EN(52, 16, "BTN2", &Font8, GUI_BACKGROUND, RED); //BTN2
	GUI_DisString_EN(77, 16, "BTN3", &Font8, GUI_BACKGROUND, RED); //BTN3

	GUI_DisString_EN(104, 4, "SW0", &Font8, GUI_BACKGROUND, WHITE); //SW0
	GUI_DisString_EN(104, 12, "SW1", &Font8, GUI_BACKGROUND, WHITE); //SW1
	GUI_DisString_EN(104, 20, "SW2", &Font8, GUI_BACKGROUND, WHITE); //SW2
	GUI_DisString_EN(104, 28, "SW3", &Font8, GUI_BACKGROUND, WHITE); //SW3

	GUI_DisString_EN(0, 26, "BOOSTER PACK", &Font12, GUI_BACKGROUND, WHITE); //Seccion Booster

	GUI_DisString_EN(0, 38, "EjeX", &Font8, GUI_BACKGROUND, WHITE); //Acelerometro X
	GUI_DisString_EN(0, 47, "EjeY", &Font8, GUI_BACKGROUND, WHITE); //Acelerometro Y
	GUI_DisString_EN(0, 56, "EjeZ", &Font8, GUI_BACKGROUND, WHITE); //Acelerometro Z

	GUI_DisString_EN(48, 40, "T", &Font8, GUI_BACKGROUND, WHITE); //Temperatura
	GUI_DisString_EN(48, 48, "Mic", &Font8, GUI_BACKGROUND, WHITE); //Microfono
	GUI_DisString_EN(48, 56, "Luz", &Font8, GUI_BACKGROUND, WHITE); //Luz

	GUI_DisString_EN(106, 40, "BTN0", &Font8, GUI_BACKGROUND, RED); //BTN0
	GUI_DisString_EN(106, 48, "BTN1", &Font8, GUI_BACKGROUND, RED); //BTN1
	GUI_DisString_EN(106, 56, "B JS", &Font8, GUI_BACKGROUND, RED); //BTN JS

	GUI_DisString_EN(0, 68, "Joystick", &Font12, GUI_BACKGROUND, WHITE); //Joystick

	GUI_DisString_EN(0, 80, "X actual", &Font8, GUI_BACKGROUND, WHITE); //Valores Joystick
	GUI_DisString_EN(0, 88, "Y actual", &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(0, 96, "X minimo", &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(0, 104, "X maximo", &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(0, 112, "Y minimo", &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(0, 120, "Y maximo", &Font8, GUI_BACKGROUND, WHITE);

	GUI_DisString_EN(82, 68, "POTs", &Font12, GUI_BACKGROUND, WHITE); //Potenciometro

	GUI_DisString_EN(64, 80, "I actual", &Font8, GUI_BACKGROUND, WHITE); //Valores Potenciometro
	GUI_DisString_EN(64, 88, "D actual", &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(64, 96, "I minimo", &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(64, 104, "I maximo", &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(64, 112, "D minimo", &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(64, 120, "D maximo", &Font8, GUI_BACKGROUND, WHITE);

	Xil_Out32(XPAR_AXI_CONNECTIONS_0_S00_AXI_BASEADDR, 0xFFFFFFFF); //Encender LED RGB
	Xil_Out32(XPAR_AXI_CONNECTIONS_0_S00_AXI_BASEADDR + 4, 0xFFFFFFFF); // Encender LEDs


	XScuGic_Connect(&GIC, INT_ID_BTN_Z0, (Xil_InterruptHandler) btn_z0_fast_mode, NULL);
	XScuGic_Enable(&GIC, INT_ID_BTN_Z0);
	XScuGic_Connect(&GIC, INT_ID_BTN_Z1, (Xil_InterruptHandler) btn_z1_fast_mode, NULL);
	XScuGic_Enable(&GIC, INT_ID_BTN_Z1);
	XScuGic_Connect(&GIC, INT_ID_BTN_Z2, (Xil_InterruptHandler) btn_z2_fast_mode, NULL);
	XScuGic_Enable(&GIC, INT_ID_BTN_Z2);
	XScuGic_Connect(&GIC, INT_ID_BTN_Z3, (Xil_InterruptHandler) btn_z3_fast_mode, NULL);
	XScuGic_Enable(&GIC, INT_ID_BTN_Z3);
	XScuGic_Connect(&GIC, INT_ID_BTN_B0, (Xil_InterruptHandler) btn_b0_fast_mode, NULL);
	XScuGic_Enable(&GIC, INT_ID_BTN_B0);
	XScuGic_Connect(&GIC, INT_ID_BTN_B1, (Xil_InterruptHandler) btn_b1_fast_mode, NULL);
	XScuGic_Enable(&GIC, INT_ID_BTN_B1);
	XScuGic_Connect(&GIC, INT_ID_BTN_BJS, (Xil_InterruptHandler) btn_bjs_fast_mode, NULL);
	XScuGic_Enable(&GIC, INT_ID_BTN_BJS);
	//XScuGic_Connect($GIC, INT_ID_TMR, (Xil_Interrupt_Handler) quick_peripherals, NULL); // Ver bien como se manejan los interrupts del axi timer

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

		GUI_DisString_EN(120, 4, sw0_, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
		GUI_DisString_EN(120, 12, sw1_, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
		GUI_DisString_EN(120, 20, sw2_, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
		GUI_DisString_EN(120, 28, sw3_, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);

		sprintf(sw0_, "%d", sw0);
		sprintf(sw1_, "%d", sw1);
		sprintf(sw2_, "%d", sw2);
		sprintf(sw3_, "%d", sw3);


		GUI_DisString_EN(120, 4, sw0_, &Font8, GUI_BACKGROUND, WHITE);
		GUI_DisString_EN(120, 12, sw1_, &Font8, GUI_BACKGROUND, WHITE);
		GUI_DisString_EN(120, 20, sw2_, &Font8, GUI_BACKGROUND, WHITE);
		GUI_DisString_EN(120, 28, sw3_, &Font8, GUI_BACKGROUND, WHITE);

		delay_ms(50);

	}
	return 0;
}

void btn_b0_inicio(void *CallbackRef){
	booster_0 = 1;
	XScuGic_Disable(&GIC, INT_ID_BTN_B0);
	XScuGic_Disable(&GIC, INT_ID_BTN_B1);
	XScuGic_Disable(&GIC, INT_ID_TMR);
}

void btn_b1_inicio(void *CallbackRef){
	booster_1 = 1;
	XScuGic_Disable(&GIC, INT_ID_BTN_B0);
	XScuGic_Disable(&GIC, INT_ID_BTN_B1);
	XScuGic_Disable(&GIC, INT_ID_TMR);
}

void btn_z0_fast_mode(void *CallbackRef){
	GUI_DisString_EN(2, 16, "BTN0", &Font8, GUI_BACKGROUND, GREEN); //BTN0
	XScuGic_Disable(&GIC, INT_ID_BTN_Z0);
}

void btn_z1_fast_mode(void *CallbackRef){
	GUI_DisString_EN(27, 16, "BTN1", &Font8, GUI_BACKGROUND, GREEN); //BTN1
	XScuGic_Disable(&GIC, INT_ID_BTN_Z1);
}

void btn_z2_fast_mode(void *CallbackRef){
	GUI_DisString_EN(52, 16, "BTN2", &Font8, GUI_BACKGROUND, GREEN); //BTN2
	XScuGic_Disable(&GIC, INT_ID_BTN_Z2);
}

void btn_z3_fast_mode(void *CallbackRef){
	GUI_DisString_EN(77, 16, "BTN3", &Font8, GUI_BACKGROUND, GREEN); //BTN3
	XScuGic_Disable(&GIC, INT_ID_BTN_Z3);
}

void btn_b0_fast_mode(void *CallbackRef){
	GUI_DisString_EN(106, 40, "BTN0", &Font8, GUI_BACKGROUND, GREEN); //BTN0
	XScuGic_Disable(&GIC, INT_ID_BTN_B0);
}

void btn_b1_fast_mode(void *CallbackRef){
	GUI_DisString_EN(106, 48, "BTN1", &Font8, GUI_BACKGROUND, GREEN); //BTN1
	XScuGic_Disable(&GIC, INT_ID_BTN_B1);
}

void btn_bjs_fast_mode(void *CallbackRef){
	GUI_DisString_EN(106, 56, "B JS", &Font8, GUI_BACKGROUND, GREEN); //BTN JS
	XScuGic_Disable(&GIC, INT_ID_BTN_BJS);
}

void quick_peripherals(void *CallbackRef){
	int joyx_int;
	int joyy_int;

	int pot1_int;
	int pot2_int;

	GUI_DisString_EN(40, 80, joyx, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
	GUI_DisString_EN(40, 88, joyy, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);

	GUI_DisString_EN(104, 80, pot1, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
	GUI_DisString_EN(104, 88, pot2, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);

	GUI_DisString_EN(24, 38, acx, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
	GUI_DisString_EN(24, 47, acy, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
	GUI_DisString_EN(24, 56, acz, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);

	GUI_DisString_EN(64, 40, mic, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
	GUI_DisString_EN(64, 48, temp, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
	GUI_DisString_EN(64, 56, luz, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);

	joyx_int = read_joyx();
	if (joyx_int < joyx_min){
		GUI_DisString_EN(40, 96, jx_min, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
		joyx_min = joyx_int;
		sprintf(jx_min, "%d", joyx_min);
		GUI_DisString_EN(40, 96, jx_min, &Font8, GUI_BACKGROUND, WHITE);
	}
	else if (joyx_int > joyx_max){
		GUI_DisString_EN(40, 104, jx_max, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
		joyx_max = joyx_int;
		sprintf(jx_max, "%d", joyx_max);
		GUI_DisString_EN(40, 104, jx_max, &Font8, GUI_BACKGROUND, WHITE);
	}

	joyy_int = read_joyy();
	if (joyy_int < joyy_min){
		GUI_DisString_EN(40, 112, jy_min, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
		joyy_min = joyy_int;
		sprintf(jy_min, "%d", joyy_min);
		GUI_DisString_EN(40, 112, jy_min, &Font8, GUI_BACKGROUND, WHITE);
	}
	else if (joyy_int > joyy_max){
		GUI_DisString_EN(40, 120, jy_max, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
		joyy_max = joyy_int;
		sprintf(jy_max, "%d", joyy_max);
		GUI_DisString_EN(40, 120, jy_max, &Font8, GUI_BACKGROUND, WHITE);
	}
	sprintf(joyx, "%d", joyx_int);
	sprintf(joyy, "%d", joyy_int);
	GUI_DisString_EN(40, 80, joyx, &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(40, 88, joyy, &Font8, GUI_BACKGROUND, WHITE);

	pot1_int = read_POT1();
	if (pot1_int < pot1_min){
		GUI_DisString_EN(104, 96, p1_min, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
		pot1_min = pot1_int;
		sprintf(p1_min, "%d", pot1_min);
		GUI_DisString_EN(104, 96, p1_min, &Font8, GUI_BACKGROUND, WHITE);
	}
	else if (pot1_int > pot1_max){
		GUI_DisString_EN(104, 104, p1_max, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
		pot1_max = pot1_int;
		sprintf(p1_max, "%d", pot1_max);
		GUI_DisString_EN(104, 104, p1_max, &Font8, GUI_BACKGROUND, WHITE);
	}

	pot2_int = read_POT2();
	if (pot2_int < pot2_min){
		GUI_DisString_EN(104, 112, p2_min, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
		pot2_min = pot2_int;
		sprintf(p2_min, "%d", pot2_min);
		GUI_DisString_EN(104, 112, p2_min, &Font8, GUI_BACKGROUND, WHITE);
	}
	else if (pot2_int > pot2_max){
		GUI_DisString_EN(104, 120, p2_max, &Font8, GUI_BACKGROUND, GUI_BACKGROUND);
		pot2_max = pot2_int;
		sprintf(p2_max, "%d", pot2_max);
		GUI_DisString_EN(104, 120, p2_max, &Font8, GUI_BACKGROUND, WHITE);
	}

	sprintf(pot1, "%d", pot1_int);
	sprintf(pot2, "%d", pot2_int);
	GUI_DisString_EN(104, 80, pot1, &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(104, 88, pot2, &Font8, GUI_BACKGROUND, WHITE);

	sprintf(acx, "%d", read_acx());
	sprintf(acy, "%d", read_acy());
	sprintf(acz, "%d", read_acz());

	GUI_DisString_EN(24, 38, acx, &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(24, 47, acy, &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(24, 56, acz, &Font8, GUI_BACKGROUND, WHITE);


	sprintf(mic, "%d", read_MIC());
	sprintf(temp, "%d", read_tmp());
	sprintf(luz, "%d", read_opt());

	GUI_DisString_EN(64, 40, mic, &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(64, 48, temp, &Font8, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(64, 56, luz, &Font8, GUI_BACKGROUND, WHITE);

}
