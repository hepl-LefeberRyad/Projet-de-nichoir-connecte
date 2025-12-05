#include "M5TimerCAM.h"

const int LED_PIN = 4;      // LED externe sur GPIO4
const int sensor_PIN = 13;  // Capteur Pir 5578 BS-612 sur GPIO13

void setup() {
    // Configurer le bouton comme entrée avec pull-up
    pinsensor, INPUT_PULLUP);

    // Autoriser le réveil par bouton AVANT toute init
    esp_sleep_enable_ext0_wakeup((gpio_nsensor, 1); // réveil sur niveau bas

    // Initialisation du TimerCAM
    TimerCAM.begin(true);

    pinMode(LED_PIN, OUTPUT);

    Serial.begin(115200);
    Serial.println("Wake up!!!");

    // LED toujours allumée
    analogWrite(LED_PIN, 255);
    vTaskDelay(pdMS_TO_TICKS(10000)); // 10 000 ms = 10 secondes


    // Autoriser le réveil par le capteur aussi après le mode sleep 
    esp_sleep_enable_ext0_wakeup((gpio_nsensor, 1);
    Serial.println("sleep!!");
    // Mise en veille après 5 secondes
    TimerCAM.Power.timerSleep(5);
 
}

void loop() {
}
