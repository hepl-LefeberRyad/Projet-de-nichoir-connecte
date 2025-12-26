#include "M5TimerCAM.h"
#include <esp_sleep.h>
#include "esp32/rtc.h"

const int LED_PIN = 4;        // LED externe GPIO4
const int SENSOR_PIN = 13;    // PIR sur GPIO13

// 1 minute en microsecondes
#define PERIOD_US (60ULL * 1000000ULL)

// Variable RTC persistante (ne disparaît pas en deep-sleep)
RTC_DATA_ATTR uint64_t next_wakeup = 0;

void setup() {

    Serial.begin(115200);
    delay(500);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    pinMode(SENSOR_PIN, INPUT_PULLUP);

    // Initialisation TimerCAM
    TimerCAM.begin(true);

    // Lire le compteur RTC permanent (microsecondes)
    uint64_t now = esp_rtc_get_time_us();

    // Identifier la cause du réveil
    esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();

    if (reason == ESP_SLEEP_WAKEUP_EXT0) {
        Serial.println("Reveil par capteur PIR");

        digitalWrite(LED_PIN, HIGH);
        delay(10000);
        digitalWrite(LED_PIN, LOW);

    } else if (reason == ESP_SLEEP_WAKEUP_TIMER) {
        Serial.println("Reveil par timer permanent");

    } else {
        Serial.println("Premier demarrage / reset");

        // Première initialisation du timer permanent
        next_wakeup = now + PERIOD_US;
    }

    // Recalculer la durée restante avant la prochaine échéance
    now = esp_rtc_get_time_us();

    uint64_t remaining;
    if (now >= next_wakeup) {
        // Si on a dépassé l’échéance, on programme la suivante
        next_wakeup = now + PERIOD_US;
        remaining = PERIOD_US;
    } else {
        remaining = next_wakeup - now;
    }

    // === Réactiver les sources de réveil ===

    // Réveil par PIR (niveau HIGH)
    esp_sleep_enable_ext0_wakeup((gpio_num_t)SENSOR_PIN, 1);

    // Réveil par timer RTC (durée restante)
    esp_sleep_enable_timer_wakeup(remaining);

    Serial.println("sleep...");
    esp_deep_sleep_start();
}

void loop() {
    // Jamais exécuté
}