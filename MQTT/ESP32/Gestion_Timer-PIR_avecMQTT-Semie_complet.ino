#include "M5TimerCAM.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_sleep.h>
#include "esp32/rtc.h"
#include "base64.h"

// ================= GPIO ==================
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
#define MQTT_TOPIC_PIC "pics"

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

// ================= CAPTURE PHOTO AND SEND IN CHUNKS =========
void sendPhotoInChunks() {
    if (!TimerCAM.Camera.get()) {
        Serial.println("Error capturing image");
        return;
    }

    uint8_t* buf = TimerCAM.Camera.fb->buf;
    size_t len = TimerCAM.Camera.fb->len;

    Serial.printf("Image captured (%d bytes)\n", len);

    size_t chunkSize = 4095;
    int totalChunks = (len + chunkSize - 1) / chunkSize;

    String imageID = String(millis());

    for (int i = 0; i < totalChunks; i++) {
        size_t start = i * chunkSize;
        size_t sz = min(chunkSize, len - start);

        String base64Chunk = base64::encode(buf + start, sz);
        String payload = imageID + "|" + String(i) + "|" + String(totalChunks) + "|" + base64Chunk;

        bool ok = mqttClient.publish(MQTT_TOPIC_PIC, payload.c_str());
        Serial.println(ok ? "Chunk sent" : "Failed to send chunk");
        delay(10);
    }

    TimerCAM.Camera.free();
}

void sendPhotoTakenNotification() {
    bool ok = mqttClient.publish(MQTT_TOPIC_PIC, "Photo taken");
    Serial.println(ok ? "MQTT notification sent" : "MQTT publish failed");
}

// ================= SETUP ==================
void setup() {

    Serial.begin(115200);
    delay(500);

    pinMode(SENSOR_PIN, INPUT_PULLUP);

    TimerCAM.begin(true);

    uint64_t now = esp_rtc_get_time_us();
    esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();

    // ================= PIR WAKEUP =================
    if (reason == ESP_SLEEP_WAKEUP_EXT0) {
        Serial.println("Reveil par PIR");

        // Réinitialisation de la fenêtre de 10 minutes
        if (window_start == 0 || (now - window_start) > WINDOW_US) {
            window_start = now;
            pir_count = 0;
        }

        // Vérifier si on peut encore prendre une photo
        if (pir_count < 2) {

            // Vérifier que le PIR est bien haut au réveil
            if (digitalRead(SENSOR_PIN) == LOW) {
                Serial.println("PIR LOW at wake → pas de mouvement stable, retour au sommeil");
            } else {

                Serial.println("PIR detected → checking 10s continuous motion...");

                unsigned long motionStart = millis();
                bool continuousMotion = true;

                // Vérifier 10 secondes de mouvement continu
                while (millis() - motionStart < 10000) {
                    if (digitalRead(SENSOR_PIN) == LOW) {
                        continuousMotion = false;
                        break;
                    }
                    delay(50);
                }

                if (!continuousMotion) {
                    Serial.println("Motion NOT continuous for 10s → no photo");
                }
                else {
                    Serial.println("Motion for 10s → Taking photo");

                    // On incrémente SEULEMENT quand une photo est réellement prise
                    pir_count++;

                    // Init camera
                    if (!TimerCAM.Camera.begin()) {
                        Serial.println("Camera Init Fail");
                    } else {
                        TimerCAM.Camera.sensor->set_pixformat(TimerCAM.Camera.sensor, PIXFORMAT_JPEG);
                        TimerCAM.Camera.sensor->set_framesize(TimerCAM.Camera.sensor, FRAMESIZE_XGA);
                        TimerCAM.Camera.sensor->set_vflip(TimerCAM.Camera.sensor, 1);
                        TimerCAM.Camera.sensor->set_quality(TimerCAM.Camera.sensor, 3);

                        // WiFi + MQTT
                        connectWiFi();
                        connectMQTT();

                        // Capture + envoi
                        sendPhotoInChunks();
                        sendPhotoTakenNotification();

                        delay(200);
                        WiFi.disconnect(true);
                    }
                }
            }
        }
        else {
            Serial.println("Limite de 2 photos atteinte pour cette fenêtre de 10 minutes");
        }
    }

    // ================= TIMER WAKEUP =================
    else if (reason == ESP_SLEEP_WAKEUP_TIMER) {
        Serial.println("Reveil par TIMER → envoi MQTT");

        int batteryVoltage = TimerCAM.Power.getBatteryVoltage();
        int batteryLevel   = TimerCAM.Power.getBatteryLevel();

        char batteryMsg[80];
        sprintf(batteryMsg,
                "Bat Level: %d %% & Bat Voltage: %d mV",
                batteryLevel,
                batteryVoltage);

        Serial.println(batteryMsg);

        connectWiFi();
        connectMQTT();

        mqttClient.publish(MQTT_TOPIC_BATTERY, batteryMsg);
        mqttClient.loop();

        delay(200);
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