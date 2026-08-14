/*
 * led_purple.h
 *
 * Public API for the light-purple fading LED driver.
 *
 * Call led_purple_start() after Wi-Fi is up (it doesn't interfere with anything
 * else).  The LED will breathe light-purple (R≈170, G=0, B≈170) with a ~2s cycle.
 */

#ifndef LED_PURPLE_H
#define LED_PURPLE_H

#ifdef __cplusplus
extern "C" {
#endif

void led_purple_start(void);
void led_purple_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* LED_PURPLE_H */