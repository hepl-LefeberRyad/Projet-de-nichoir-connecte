#include "M5TimerCAM.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "base64.h"
 
// ================= WIFI ==================
#define WIFI_SSID "VOO-2G7LV36"
#define WIFI_PASSWORD "cpCafHXHaGYGGLsxgf"
 
// ================= MQTT ==================
#define MQTT_HOST "192.168.0.194"
#define MQTT_PORT 1883
#define MQTT_TOPIC "pics"
 
 
 
// PIR Sensor
int sensorPin = 13;
int sensorState = LOW;
unsigned long motionStart = 0;
bool photoTaken = false;
 
// MQTT client
WiFiClient espClient;
PubSubClient mqttClient(espClient);
 
// ================= SETUP =====================
void setup() {
    Serial.begin(115200);
    pinMode(sensorPin, INPUT);
 
    // Init TimerCAM
    TimerCAM.begin();
    if (!TimerCAM.Camera.begin()) {
        Serial.println("Camera Init Fail");
        return;
    }
    Serial.println("Camera Ready");
 
    // Camera format & resolution
TimerCAM.Camera.sensor->set_pixformat(TimerCAM.Camera.sensor, PIXFORMAT_JPEG);
TimerCAM.Camera.sensor->set_framesize(TimerCAM.Camera.sensor, FRAMESIZE_XGA); // or UXGA
TimerCAM.Camera.sensor->set_vflip(TimerCAM.Camera.sensor, 1);
TimerCAM.Camera.sensor->set_quality(TimerCAM.Camera.sensor, 3); // 0-63, lower = better
 
 
    // WiFi
    setupWiFi();
 
    // MQTT
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    connectToMQTT();
}
 
// ================= WIFI =====================
void setupWiFi() {
    WiFi.disconnect(true);
    delay(1000);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
 
    Serial.print("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - startAttemptTime < 10000) {
        delay(500);
        Serial.print(".");
    }
 
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(" Failed!");
        return;
    }
 
    Serial.println(" Connected!");
    Serial.print("IP: "); Serial.println(WiFi.localIP());
}
 
// ================= MQTT =====================
void connectToMQTT() {
    if (WiFi.status() != WL_CONNECTED) return;
 
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect("M5TimerCAM")) {
        Serial.println("Connected!");
    } else {
        Serial.print("Failed, rc=");
        Serial.println(mqttClient.state());
    }
}
 
void mqttReconnectIfNeeded() {
    if (!mqttClient.connected()) {
        connectToMQTT();
    }
    mqttClient.loop();
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
 
    size_t chunkSize = 4095; // 4 KB chunks
    int totalChunks = (len + chunkSize - 1) / chunkSize;
 
    String imageID = String(millis()); // unique image ID
 
    for (int i = 0; i < totalChunks; i++) {
        size_t start = i * chunkSize;
        size_t sz = min(chunkSize, len - start);
 
        String base64Chunk = base64::encode(buf + start, sz);
        String payload = imageID + "|" + String(i) + "|" + String(totalChunks) + "|" + base64Chunk;
 
        if (!mqttClient.connected()) mqttReconnectIfNeeded();
        bool ok = mqttClient.publish(MQTT_TOPIC, payload.c_str());
        Serial.println(ok ? "Chunk sent" : "Failed to send chunk");
        delay(10);
    }
 
    TimerCAM.Camera.free();
}
 
// ================= SEND TEXT NOTIFICATION =========
void sendPhotoTakenNotification() {
    if (!mqttClient.connected()) mqttReconnectIfNeeded();
    bool ok = mqttClient.publish(MQTT_TOPIC, "Photo taken");
    Serial.println(ok ? "MQTT notification sent" : "MQTT publish failed");
}
 
// ================= MAIN LOOP ==================
void loop() {
    mqttReconnectIfNeeded();
 
    int motion = digitalRead(sensorPin);
 
    if (motion == HIGH) {
        if (sensorState == LOW) {
            motionStart = millis();
            photoTaken = false;
            Serial.println("Motion detected...");
            sensorState = HIGH;
        }
        else if (!photoTaken && (millis() - motionStart >= 10000)) {
            Serial.println("Motion for 10s → Taking photo");
 
            sendPhotoInChunks();          // Send image in chunks
            sendPhotoTakenNotification(); // Optional notification
 
            photoTaken = true;
        }
    }
    else if (sensorState == HIGH) {
        Serial.println("Motion ended.");
        sensorState = LOW;
    }
 
    delay(100);
}
