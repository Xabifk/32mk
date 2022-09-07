/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 * Copyright 2018 Gal Zaidenstein.
 */

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "key_definitions.h"
#include "keyboard_config.h"
#include "key_definitions.h"
#include "driver/pcnt.h"
#include "hal_ble.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "keyboard_config.h"
#include "joystick.h"

static const adc_channel_t channel_x = JOYSTICK_X_AXIS;
static const adc_channel_t channel_y = JOYSTICK_Y_AXIS;
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

static esp_adc_cal_characteristics_t *adc_chars_x;
static esp_adc_cal_characteristics_t *adc_chars_y;

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   5          //Multisampling

//How to process encoder activity
void joystick_command(struct joystick_state_t state) {
	int16_t mouse_state[5] = {0};
	if (state.pressed == 1) {
		mouse_state[0] = 4;
	}
	int8_t speed_lookup[] = {0, 0, 1, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 7, 8, 9, 10, 11, 11, 11, 11, 11};
	int reverse[] = {0, 1, -1};
 
	mouse_state[1] = state.y_axis;
	mouse_state[2] = state.x_axis;

	int range_size = 0;
	int sign = 0;


	for (int i = 1; i <= 2; i++) {
		mouse_state[i] -= 1850;
		if (mouse_state[i] < 0) {
			range_size = 1850 / 22;
			sign = -1;
		}
		else {
			range_size = (4096 - 1850) / 22;
			sign = 1;
		}
		mouse_state[i] = abs(mouse_state[i]);

		mouse_state[i] = speed_lookup[mouse_state[i] / range_size] * sign * reverse[i];
	}

	//printf("Mouse Axis 1:%d Mouse Axis 2:%d Mouse Button: %d\n", mouse_state[1], mouse_state[2], mouse_state[0]);

	int8_t final_mouse_state[5] = {mouse_state[0], mouse_state[1], mouse_state[2], 0, 0};


	xQueueSend(mouse_q,(void*)&final_mouse_state, (TickType_t) 0);
	vTaskDelay(5 / portTICK_PERIOD_MS);
}
//Setting Pulse counter and encoder button
void joystick_setup(void){

	adc1_config_width(ADC_WIDTH_BIT_12);

	adc1_config_channel_atten(JOYSTICK_X_AXIS, atten);
	adc_chars_x = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF,
			adc_chars_x);

	adc1_config_channel_atten(JOYSTICK_Y_AXIS, atten);
	adc_chars_y = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF,
			adc_chars_y);
	

	gpio_pad_select_gpio(JOYSTICK_SWITCH);
	gpio_set_direction(JOYSTICK_SWITCH, GPIO_MODE_INPUT);
	gpio_set_pull_mode(JOYSTICK_SWITCH, GPIO_PULLUP_ONLY);
}

//Check encoder state, currently defined for Vol +/= and mute
struct joystick_state_t joystick_state(void) {
	struct joystick_state_t state;

	uint32_t adc_reading_x = 0;
	uint32_t adc_reading_y = 0;
	//Multisampling

	for (int i = 0; i < NO_OF_SAMPLES; i++) {
		adc_reading_x += adc1_get_raw((adc1_channel_t) channel_x);
		adc_reading_y += adc1_get_raw((adc1_channel_t) channel_y);
	}
	adc_reading_x /= NO_OF_SAMPLES;
	adc_reading_y /= NO_OF_SAMPLES;

	state.x_axis = adc_reading_x;
	state.y_axis = adc_reading_y;
	if(gpio_get_level(JOYSTICK_SWITCH) == 0){
		state.pressed = 1;
	}
	else {
		state.pressed = 0;
	}


	return state;

}





