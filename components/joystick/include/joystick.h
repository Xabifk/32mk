/*
 * r_encoder.h
 *
 *  Created on: 18 Aug 2018
 *      Author: gal
 */

#ifndef JOYSTICK_H_
#define JOYSTICK_H_

struct joystick_state_t{
    uint16_t x_axis;
    uint16_t y_axis;
    uint8_t pressed;
};

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Setup the rotary encoder
 * */
void joystick_setup(void);

/** @brief check rotary encoder status
 * */
struct joystick_state_t joystick_state(void);

void joystick_command(struct joystick_state_t state);

#ifdef __cplusplus
}
#endif

#endif /* R_ENCODER_H_ */
