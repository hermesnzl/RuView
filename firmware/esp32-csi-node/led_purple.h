/*
 * led_purple.h
 *
 * Public interface for the light-purple fading LED driver.
 *
 * Call led_purple_start() after you have created the led_strip device.
 * Pass the same handle you gave led_strip_new_rmt_device():
 *
 *   led_strip_handle_t strip;
 *   led_strip_new_rmt_device(&strip_config, &rmt_config, &strip);
 *   led_purple_start(strip);
 */
#ifndef LED_PURPLE_H
#define LED_PURPLE_H

#include "driver/led_strip.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Start the smooth light-purple LED fade using the provided led_strip handle. */
void led_purple_start(led_strip_handle_t strip);

/** Stop the fading task (optional; board will clear on shutdown anyway). */
void led_purple_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* LED_PURPLE_H */