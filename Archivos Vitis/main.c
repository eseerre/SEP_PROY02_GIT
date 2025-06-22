#include <stdio.h>
#include <sleep.h>
#include <time.h>
#include <unistd.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xstatus.h"
#include "Delay.h"
#include "I2C.h"





int main()
{
	int Status;
	int TEMPERATURA;
	int LUX;

    init_platform();

    //Initialize IIC IPCore Module
	Status = init_IIC();
	if (Status != XST_SUCCESS) {
		xil_printf("IIC Mode Failed\r\n");
		return XST_FAILURE;
	}



    while(1){
		TEMPERATURA = read_tmp();                       //TEMPERATURE VARIABLE IS EQUAL TO THE RETURNED VALUE OF READ_TMP() FUNCTION
		LUX = read_opt();								//LUX VARIABLE IS EQUAL TO THE RETURNED VALUE OF READ_OPT() FUNCTION
		xil_printf("TEMPERATURA: %d\n", TEMPERATURA);   //PRINT VARIABLES
		xil_printf("LUZ: %d\n", LUX);
		xil_printf("\n");
		sleep(1);
    }
    return 0;

}
