#ifndef __I2C_H
#define __I2C_H
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xiic.h"
#define IIC_dev 			XPAR_IIC_0_DEVICE_ID				//I2C DEVIDE ID
#define TMP_ADDR			0x40								//TMP SENSOR SLAVE ADDRESS
#define OPT_ADDR			0x44								//LIGHT SENSOR SLAVE ADDRESS


int init_IIC();
int read_tmp();
int read_opt();

#endif
