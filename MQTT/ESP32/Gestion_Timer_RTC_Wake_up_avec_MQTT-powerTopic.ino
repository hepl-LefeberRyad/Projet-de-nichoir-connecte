#include "M5TimerCAM.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_sleep.h>
#include "esp32/rtc.h"

// ================= GPIO ==================
const int LED_PIN = 4;
const int SENSOR_PIN = 13;

// ================= TIMING ==================
// 1 minute
#define PERIOD_US (60ULL * 1000000ULL)
// Fenêtre PIR 10 minutes
#define WINDOW_US (10ULL * 60ULL * 1000000ULL)

// ================= WIFI ==================
#define WIFI_SSID     "VOO-2G7LV36"
#define WIFI_PASSWORD "cpCafHXHaGYGGLsxgf"

// ================= MQTT ==================
#define MQTT_HOST "192.168.0.194"
#define MQTT_PORT 1883
#define MQTT_TOPIC_BATTERY "timercam/battery"

// ================= RTC DATA ==================
RTC_DATA_ATTR uint64_t next_wakeup = 0;
RTC_DATA_ATTR uint64_t window_start = 0;
RTC_DATA_ATTR uint8_t  pir_count = 0;

// ================= MQTT OBJECTS ==================
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// ---------- WiFi ----------
void connectWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
    }
}

// ---------- MQTT ----------
void connectMQTT() {
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    while (!mqttClient.connected()) {
        mqttClient.connect("TimerCAM_Client");
        delay(500);
    }
}

void setup() {

    Serial.begin(115200);
    delay(500);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    pinMode(SENSOR_PIN, INPUT_PULLUP);

    TimerCAM.begin(true);

    uint64_t now = esp_rtc_get_time_us();
    esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();

    // ================= PIR WAKEUP =================
    if (reason == ESP_SLEEP_WAKEUP_EXT0) {
        Serial.println("Reveil par PIR");

        if (window_start == 0 || (now - window_start) > WINDOW_US) {
            window_start = now;
            pir_count = 0;
        }

        if (pir_count < 2) {
            pir_count++;
            digitalWrite(LED_PIN, HIGH);
            delay(10000);
            digitalWrite(LED_PIN, LOW);
        } else {
            Serial.println("Limite PIR atteinte");
        }
    }

    // ================= TIMER WAKEUP =================
    else if (reason == ESP_SLEEP_WAKEUP_TIMER) {
        Serial.println("Reveil par TIMER → envoi MQTT");

        // Lecture batterie
        int batteryVoltage = TimerCAM.Power.getBatteryVoltage();
        int batteryLevel   = TimerCAM.Power.getBatteryLevel();

        // Message unique
        char batteryMsg[80];
        sprintf(batteryMsg,
                "Bat Level: %d %% & Bat Voltage: %d mV",
                batteryLevel,
                batteryVoltage);

        Serial.println(batteryMsg);

        // WiFi + MQTT UNIQUEMENT ICI
        connectWiFi();
        connectMQTT();

        mqttClient.publish(MQTT_TOPIC_BATTERY, batteryMsg);
        mqttClient.loop();

        delay(200); // laisse partir le paquet
        WiFi.disconnect(true);
    }

    // ================= FIRST BOOT =================
    else {
        Serial.println("Premier demarrage");
        next_wakeup = now + PERIOD_US;
        window_start = now;
        pir_count = 0;
    }

    // ================= NEXT WAKEUP =================
    now = esp_rtc_get_time_us();
    uint64_t remaining = (now >= next_wakeup) ? PERIOD_US : (next_wakeup - now);
    next_wakeup = now + remaining;

    esp_sleep_enable_ext0_wakeup((gpio_num_t)SENSOR_PIN, 1);
    esp_sleep_enable_timer_wakeup(remaining);

    Serial.println("Sleep...");
    esp_deep_sleep_start();
}

void loop() {}
