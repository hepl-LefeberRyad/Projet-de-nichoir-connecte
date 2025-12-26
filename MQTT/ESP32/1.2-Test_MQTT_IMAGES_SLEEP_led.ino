#include "M5TimerCAM.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "base64.h"

// ================= WIFI ===================
#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASSWORD "WIFI_PASSWORD"

// ================= MQTT ===================
#define MQTT_HOST "192.168.0.194"
#define MQTT_PORT 1883
#define MQTT_TOPIC "pics"

// PIR Sensor
const int sensorPin = 13;
int sensorState = LOW;
unsigned long motionStart = 0;
bool photoTaken = false;

// LED externe sur GPIO4
const int LED_PIN = 4;

// MQTT client
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// ================= SETUP =======================
void setup() {
    Serial.begin(115200);

    pinMode(sensorPin, INPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); // LED éteinte au démarrage

    // Init TimerCAM
    TimerCAM.begin(true);
    if (!TimerCAM.Camera.begin()) {
        Serial.println("Camera Init Fail");
        return;
    }
    Serial.println("Camera Ready");

    // Camera format & resolution
    TimerCAM.Camera.sensor->set_pixformat(TimerCAM.Camera.sensor, PIXFORMAT_JPEG);
    TimerCAM.Camera.sensor->set_framesize(TimerCAM.Camera.sensor, FRAMESIZE_XGA);
    TimerCAM.Camera.sensor->set_vflip(TimerCAM.Camera.sensor, 1);
    TimerCAM.Camera.sensor->set_quality(TimerCAM.Camera.sensor, 3);

    mqttClient.setServer(MQTT_HOST, MQTT_PORT);

    Serial.println("Wake up!!!");
}

// ================= WIFI ======================
void setupWiFi() {
    WiFi.disconnect(true);
    delay(200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(" Connected!");
        Serial.print("IP: "); Serial.println(WiFi.localIP());
    } else {
        Serial.println(" WiFi Failed!");
    }
}

// ================= MQTT ========================
void connectToMQTT() {
    if (WiFi.status() != WL_CONNECTED) return;
    if (mqttClient.connect("M5TimerCAM")) {
        Serial.println("MQTT Connected!");
    } else {
        Serial.print("MQTT Failed, rc=");
        Serial.println(mqttClient.state());
    }
}

// ================= CAPTURE PHOTO AND SEND =========
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

        if (!mqttClient.connected()) connectToMQTT();
        mqttClient.publish(MQTT_TOPIC, payload.c_str());
        delay(10);
    }

    TimerCAM.Camera.free();
}

// ================= MAIN LOOP ========================
void loop() {
    int motion = digitalRead(sensorPin);

       // Si le capteur capte un corps il est en état high    
    if (motion == HIGH) {
        if (sensorState == LOW) {
            motionStart = millis();
            photoTaken = false;
            Serial.println("Motion detected...");
            sensorState = HIGH;
        }
        else if (!photoTaken && (millis() - motionStart >= 10000)) {
            Serial.println("Motion for 10s → Taking photo");
  // Lorsque le capteur capte un corps pendant une periode supérieure a 10secondes alors une photo est prise 
            // Allumer la LED juste avant la photo
            digitalWrite(LED_PIN, HIGH);

            setupWiFi();
            connectToMQTT();
            // Photo prise avec succès
            sendPhotoInChunks();
            mqttClient.publish(MQTT_TOPIC, "Photo taken");

            // Éteindre la LED 2 secondes après la photo
            delay(2000);
            digitalWrite(LED_PIN, LOW);

            photoTaken = true;

            // Nettoyage et mise en veille
            WiFi.disconnect(true);
            WiFi.mode(WIFI_OFF);

            // Attendre que le PIR soit LOW avant de dormir
            Serial.println("Waiting PIR to go LOW before sleep...");
            unsigned long startWait = millis();
            while (digitalRead(sensorPin) == HIGH && millis() - startWait < 10000) {
                delay(100);
            }

            // Configurer le wakeup juste avant le sleep
            esp_sleep_enable_ext0_wakeup((gpio_num_t)sensorPin, 1);

            Serial.println("Going to sleep...");
            TimerCAM.Power.timerSleep(20); // deep sleep 20s
        }
    }
    else if (sensorState == HIGH) {
        Serial.println("Motion ended.");
        sensorState = LOW;
    }

    delay(100);
}
