#include "I2C.h"
XIic  iic;
u8 SendBuffer[2];
u8 RecvBuffer[2];
u8 config[3];
u16 Lux;
int16_t val;
int temp;
int Start = 1;
int init_IIC() {

		XIic_Config *iic_conf;

	    init_platform();

	    iic_conf = XIic_LookupConfig(IIC_dev);
	    XIic_CfgInitialize(&iic, iic_conf, iic_conf->BaseAddress);
	    XIic_Start(&iic);


		SendBuffer[0] = 0xfe;
		XIic_Send(iic.BaseAddress,TMP_ADDR,(u8 *)&SendBuffer, 1,XIIC_REPEATED_START);
		XIic_Recv(iic.BaseAddress,TMP_ADDR,(u8 *)&RecvBuffer, 2,XIIC_STOP);


		SendBuffer[0] = 0x02;
		XIic_Send(iic.BaseAddress,TMP_ADDR,(u8 *)&SendBuffer, 1,XIIC_STOP);

		SendBuffer[0] = 0x80;
		XIic_Send(iic.BaseAddress,TMP_ADDR,(u8 *)&SendBuffer, 1,XIIC_REPEATED_START);
		SendBuffer[0] = 0x82;
		XIic_Send(iic.BaseAddress,TMP_ADDR,(u8 *)&SendBuffer, 1,XIIC_REPEATED_START);



    return XST_SUCCESS;
}

int read_tmp(){
	SendBuffer[0] = 0x01; 																//TEMPERATURE MEASUREMENT REGISTER ADDRESS
	XIic_Send(iic.BaseAddress, TMP_ADDR, (u8 *)&SendBuffer, 1, XIIC_REPEATED_START);	//SEND FROM ZYBO TO BOOSTER THE TEMPERATURE REGISTER THROUGH SDA BUS
	XIic_Recv(iic.BaseAddress, TMP_ADDR, (u8 *)&RecvBuffer, 2, XIIC_STOP);				//RECIEVE THE TMP MEASURED FROM BOOSTER
	val = (RecvBuffer[0] << 8) | (RecvBuffer[1]);
	temp = val / 128;																	//CELSIUS DEGREES
	return temp;
}

int read_opt(){
	u8 config[3] = {0x01, 0xc4, 0x10};													//0X01 CONFIGURATION REGISTER
	if(Start == 1){																		//0xc410 16bits MESSAGE TO SET THE CONFIGURATION OF LIGHT SENSOR
		XIic_Send(iic.BaseAddress,OPT_ADDR,(u8 *)&config, 3, XIIC_STOP);				//LIGHT SENSOR CONFIGURATION
		Start = 0;
	}
	SendBuffer[0] = 0x00;																//RESULT REGISTER
	XIic_Send(iic.BaseAddress,OPT_ADDR,(u8 *)&SendBuffer, 1, XIIC_REPEATED_START);
	XIic_Recv(iic.BaseAddress,OPT_ADDR,(u8 *)&RecvBuffer, 2, XIIC_STOP);
	Lux = ((RecvBuffer[0])*256 + (RecvBuffer[1]));										//RESULT REGISTER CONVERSION

	return Lux;
}
