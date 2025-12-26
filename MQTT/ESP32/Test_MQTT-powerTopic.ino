#include "M5TimerCAM.h"
#include <WiFi.h>
#include <PubSubClient.h>

// ================= WIFI ==================
#define WIFI_SSID     "SSID_WIFI"
#define WIFI_PASSWORD "PASSWORD"

// ================= MQTT ==================
#define MQTT_HOST "192.168.0.194"
#define MQTT_PORT 1883

#define MQTT_TOPIC_BATTERY "timercam/battery"

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// ---------- Connexion WiFi ----------
void connectWiFi() {
    Serial.print("Connexion WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connecté");
    Serial.print("IP ESP32 : ");
    Serial.println(WiFi.localIP());
}

// ---------- Connexion MQTT ----------
void connectMQTT() {
    while (!mqttClient.connected()) {
        Serial.print("Connexion MQTT... ");
        if (mqttClient.connect("TimerCAM_Client")) {
            Serial.println("OK");
        } else {
            Serial.print("Erreur, rc=");
            Serial.println(mqttClient.state());
            delay(2000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    TimerCAM.begin(true);

    connectWiFi();

    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    connectMQTT();
}

void loop() {
    if (!mqttClient.connected()) {
        connectMQTT();
    }
    mqttClient.loop();

    int batteryVoltage = TimerCAM.Power.getBatteryVoltage(); // mV
    int batteryLevel   = TimerCAM.Power.getBatteryLevel();   // %

    // Affichage terminal série
    Serial.printf("Bat Level: %d %% & Bat Voltage: %d mV\r\n",
                  batteryLevel, batteryVoltage);

    // Buffer MQTT UNIQUE
    char batteryMsg[80];

    sprintf(batteryMsg,
            "Bat Level: %d %% & Bat Voltage: %d mV",
            batteryLevel,
            batteryVoltage);

    // Publication MQTT (1 topic, 1 message)
    mqttClient.publish(MQTT_TOPIC_BATTERY, batteryMsg);

    delay(1000);
}
