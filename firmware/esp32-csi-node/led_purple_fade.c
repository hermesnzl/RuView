/*
 * led_purple_fade.c
 *
 * Light‑purple fading LED driver for ESP32-C6 (Nano‑ESP32‑C6 V1711).
 *
 * Uses the same led_strip (WS2812 / RMT) API the rest of the firmware
 * already depends on, so no extra driver code is introduced.
 *
 * The on‑board LED fades smoothly from off -> light‑purple (R≈170, G=0,
 * B≈170) -> off, with a full cycle of ~2 s, on a low‑priority FreeRTOS
 * task.  It never touches the WiFi or CSI data path.
 *
 * Public API:
 *   void led_purple_start(led_strip_handle_t strip);
 *   void led_purple_stop(void);
 *
 * SPDX‑License-Identifier: MIT
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/led_strip.h"
#include <esp_log.h>
#include <math.h>

static const char *TAG = "led_purple";

/* Light‑purple target colour (0‑255 range used by the led_strip API). */
#define LED_PURPLE_R   170
#define LED_PURPLE_G     0
#define LED_PURPLE_B   170

/* Fade geometry.  100 updates/s, sine wave → one full fade ≈ 2 s. */
#define LED_UPDATE_PERIOD_MS 10
#define LED_FADE_HALF_CYCLE_S 1.0f

static TaskHandle_t s_fade_task = NULL;

static void led_purple_fade_task(void *arg)
{
    led_strip_handle_t strip = (led_strip_handle_t)arg;
    float phase = 0.0f;
    /* Advance phase per tick so a full on→off→on cycle = 2 × half cycle. */
    const float phase_inc = (2.0f * (float)M_PI) *
                            ((float)LED_UPDATE_PERIOD_MS / 1000.0f) /
                            LED_FADE_HALF_CYCLE_S;

    ESP_LOGI(TAG, "purple fade started (GPIO %d)", strip ? 0 : -1);

    for (;;) {
        /* brightness = 0.5 * (1 + sin(phase)) → 0.0 … 1.0 */
        float brightness = 0.5f * (1.0f + sinf(phase));
        /* Apply brightness to the fixed light‑purple colour. */
        uint8_t r = (uint8_t)(LED_PURPLE_R * brightness);
        uint8_t g = (uint8_t)(LED_PURPLE_G * brightness);
        uint8_t b = (uint8_t)(LED_PURPLE_B * brightness);
        led_strip_set_pixel(strip, 0, r, g, b);
        led_strip_refresh(strip);

        phase += phase_inc;
        if (phase > (2.0f * (float)M_PI)) {
            phase -= 2.0f * (float)M_PI;
        }

        vTaskDelay(pdMS_TO_TICKS(LED_UPDATE_PERIOD_MS));
    }
}

void led_purple_start(led_strip_handle_t strip)
{
    if (s_fade_task != NULL) {
        return; /* already running */
    }
    if (strip == NULL) {
        ESP_LOGW(TAG, "no led_strip handle — skipping purple fade");
        return;
    }
    BaseType_t rc = xTaskCreatePinnedToCore(
        led_purple_fade_task,
        "led_purple",
        4096,
        strip,
        3,   /* priority below CSI/WiFi */
        &s_fade_task,
        0);
    if (rc != pdPASS) {
        ESP_LOGE(TAG, "could not create fade task");
        return;
    }
    ESP_LOGI(TAG, "fade task created");
}

void led_purple_stop(void)
{
    if (s_fade_task != NULL) {
        vTaskDelete(s_fade_task);
        s_fade_task = NULL;
    }
}
