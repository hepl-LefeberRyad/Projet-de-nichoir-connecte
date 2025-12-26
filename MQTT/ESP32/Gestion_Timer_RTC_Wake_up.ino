#include "M5TimerCAM.h"
#include <esp_sleep.h>
#include "esp32/rtc.h"

const int LED_PIN = 4; // config de la led
const int SENSOR_PIN = 13; //config de la pir

// 1 minute en microsecondes
#define PERIOD_US (60ULL * 1000000ULL)

// Fenêtre de 10 minutes en microsecondes
#define WINDOW_US (10ULL * 60ULL * 1000000ULL)

// Variables persistantes en deep-sleep
RTC_DATA_ATTR uint64_t next_wakeup = 0;
RTC_DATA_ATTR uint64_t window_start = 0;
RTC_DATA_ATTR uint8_t pir_count = 0;

void setup() {

    Serial.begin(115200);
    delay(500);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    pinMode(SENSOR_PIN, INPUT_PULLUP);

    TimerCAM.begin(true);

    uint64_t now = esp_rtc_get_time_us();
    esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();

    // === Gestion du PIR ===
    if (reason == ESP_SLEEP_WAKEUP_EXT0) {
        Serial.println("Reveil par PIR");

        // Si première fois ou fenêtre expirée
        if (window_start == 0 || (now - window_start) > WINDOW_US) {
            window_start = now;
            pir_count = 0;
        }

        if (pir_count < 2) {
            pir_count++;
            Serial.printf("LED allumee (%d/2 dans la fenetre)\n", pir_count);

            digitalWrite(LED_PIN, HIGH);
            delay(10000);
            digitalWrite(LED_PIN, LOW);

        } else {
            Serial.println("Limite atteinte : LED non allumee");
        }

    } else if (reason == ESP_SLEEP_WAKEUP_TIMER) {
        Serial.println("Reveil par timer permanent");

    } else {
        Serial.println("Premier demarrage / reset");

        next_wakeup = now + PERIOD_US;
        window_start = now;
        pir_count = 0;
    }

    // === Gestion du timer permanent ===
    now = esp_rtc_get_time_us();
    uint64_t remaining;

    if (now >= next_wakeup) {
        next_wakeup = now + PERIOD_US;
        remaining = PERIOD_US;
    } else {
        remaining = next_wakeup - now;
    }

    // === Sources de réveil ===
    esp_sleep_enable_ext0_wakeup((gpio_num_t)SENSOR_PIN, 1);
    esp_sleep_enable_timer_wakeup(remaining);

    Serial.println("sleep...");
    esp_deep_sleep_start();
}

void loop() {}