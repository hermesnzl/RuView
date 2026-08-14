/*
 * led_purple_fade.c
 *
 * Light-purple fading LED driver for ESP32-C6 (Nano-ESP32-C6 V1711).
 *
 * Uses LEDC PWM on GPIO 2 (the board's built-in LED) with 12-bit resolution.
 * The LED fades smoothly between off and light-purple (R�≈170, G=0, B�≈170)
 * with a full cycle taking ~2 seconds.
 *
 * This driver is intentionally simple: it only uses ESP-IDF's LEDC peripheral.
 * It does NOT use the custom led_strip driver which is meant for WS2812 LEDs.
 *
 * Public API:
 *   void led_purple_start(void);
 *   void led_purple_stop(void);
 *
 * SPDX-License-Identifier: MIT
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/ledc.h>
#include <esp_log.h>
#include <math.h>

static const char *TAG = "led_purple";

#define LED_GPIO            2           /* GPIO connected to the on-board LED */
#define LED_PWM_CHANNEL     0           /* PWM channel */
#define LED_PWM_TIMER       0           /* Timer */
#define LED_UPDATE_PERIOD_MS 10         /* 10 ms → 100 Hz update rate (2 s full fade) */

/* Light-purple colour in 0-255 RGB: R�≈170, G=0, B�≈170 */
#define LED_PURPLE_DUTY    170

/* Forward declaration for the fade task */
static void led_purple_fade_task(void *arg);

void led_purple_start(void)
{
    esp_err_t ret;

    /* Configure the LEDC timer */
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num  = LED_PWM_TIMER,
        .duty_resolution = LEDC_TIMER_12_BIT,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ret = ledc_timer_config(&timer_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure LEDC timer: %s", esp_err_to_name(ret));
        return;
    }

    /* Configure the LEDC channel */
    ledc_channel_config_t channel_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LED_PWM_CHANNEL,
        .timer_sel  = LED_PWM_TIMER,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = LED_GPIO,
        .duty       = 0,  /* Start at 0 (off) */
        .hpoint     = 0,
    };
    ret = ledc_channel_config(&channel_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure LEDC channel: %s", esp_err_to_name(ret));
        return;
    }

    /* Start the fade task */
    xTaskCreatePinnedToCore(
        led_purple_fade_task,
        "led_purple",
        4096,
        NULL,
        3,
        NULL,
        0);  /* Core 0 */
}

void led_purple_stop(void)
{
    /* Turn off LED */
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_PWM_CHANNEL, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_PWM_CHANNEL);
}

static void led_purple_fade_task(void *arg)
{
    float phase = 0.0f;
    const float phase_inc = (2.0f * (float)M_PI) * ((float)LED_UPDATE_PERIOD_MS / 1000.0f);

    ESP_LOGI(TAG, "LED purple fade started");

    while (1) {
        /* brightness = 0.5 * (1 + sin(phase)) → 0.0 … 1.0 */
        float brightness = 0.5f * (1.0f + sinf(phase));
        uint32_t duty = (uint32_t)(LED_PURPLE_DUTY * brightness * 16.0f); /* scale to 12-bit (max 4096) */

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_PWM_CHANNEL, duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_PWM_CHANNEL);

        phase += phase_inc;
        if (phase > (2.0f * (float)M_PI)) {
            phase -= 2.0f * (float)M_PI;
        }

        vTaskDelay(pdMS_TO_TICKS(LED_UPDATE_PERIOD_MS));
    }
}