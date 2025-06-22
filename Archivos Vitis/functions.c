#include "LCD_GUI.h"
#include <stdio.h>


void GUI_IMG(int imagen [128][128]){
    for (int y = 0; y < 128; y++){
        for (int x = 0; x < 128; x ++){
            GUI_DrawPoint(x, y, imagen[y][x], DOT_PIXEL_1X1_DOT_FILL_AROUND);
        }
    }
}

void btn_b0_inicio(void *CallbackRef){
	booster_0 = 1;
	XScuGic_Disable($GIC, INT_ID_BTN_B0);
	XScuGic_Disable($GIC, INT_ID_BTN_B1);
	XScuGic_Disable($GIC, INT_ID_TMR);
}

void btn_b1_inicio(void *CallbackRef){
	booster_1 = 1;
	XScuGic_Disable($GIC, INT_ID_BTN_B0);
	XScuGic_Disable($GIC, INT_ID_BTN_B1);
	XScuGic_Disable($GIC, INT_ID_TMR);
}

void btn_z0_fast_mode(void *CallbackRef){
	GUI_DisString_EN(2, 14, "BTN0", &Font8, GUI_BACKGROUND, GREEN); //BTN0
	XScuGic_Disable($GIC, INT_ID_BTN_Z0);
}

void btn_z1_fast_mode(void *CallbackRef){
	GUI_DisString_EN(27, 14, "BTN1", &Font8, GUI_BACKGROUND, GREEN); //BTN1
	XScuGic_Disable($GIC, INT_ID_BTN_Z1);
}

void btn_z2_fast_mode(void *CallbackRef){
	GUI_DisString_EN(52, 14, "BTN2", &Font8, GUI_BACKGROUND, GREEN); //BTN2
	XScuGic_Disable($GIC, INT_ID_BTN_Z2);
}

void btn_z3_fast_mode(void *CallbackRef){
	GUI_DisString_EN(77, 14, "BTN3", &Font8, GUI_BACKGROUND, GREEN); //BTN3
	XScuGic_Disable($GIC, INT_ID_BTN_Z3);
}

void btn_b0_fast_mode(void *CallbackRef){
	GUI_DisString_EN(106, 40, "BTN0", &Font8, GUI_BACKGROUND, GREEN); //BTN0
	XScuGic_Disable($GIC, INT_ID_BTN_B0);
}

void btn_b1_fast_mode(void *CallbackRef){
	GUI_DisString_EN(106, 48, "BTN1", &Font8, GUI_BACKGROUND, GREEN); //BTN1
	XScuGic_Disable($GIC, INT_ID_BTN_B1);
}

void btn_bjs_fast_mode(void *CallbackRef){
	GUI_DisString_EN(106, 56, "B JS", &Font8, GUI_BACKGROUND, GREEN); //BTN JS
	XScuGic_Disable($GIC, INT_ID_BTN_BJS);
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
