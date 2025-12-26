#include "M5TimerCAM.h"
#include <esp_sleep.h>

const int LED_PIN = 4;        // LED externe GPIO4
const int SENSOR_PIN = 13;    // PIR sur GPIO13

void setup() {

    pinMode(SENSOR_PIN, INPUT_PULLUP);

    // Réveil par capteur (niveau bas = 0 → HIGH = 1 selon ton PIR)
    esp_sleep_enable_ext0_wakeup((gpio_num_t)SENSOR_PIN, 1);

    // Initialisation du TimerCAM
    TimerCAM.begin(true);

    pinMode(LED_PIN, OUTPUT);

    Serial.begin(115200);
    Serial.println("Wake up!!!");

    // LED ON pendant 10 secondes
    analogWrite(LED_PIN, 255);
    delay(10000);

    // Reconfigurer le réveil par capteur AVANT sleep
    esp_sleep_enable_ext0_wakeup((gpio_num_t)SENSOR_PIN, 1);

    Serial.println("sleep!!");

    // Dormir 5 secondes (deep sleep TimerCAM)
    TimerCAM.Power.timerSleep(5);
}

void loop() {
}
