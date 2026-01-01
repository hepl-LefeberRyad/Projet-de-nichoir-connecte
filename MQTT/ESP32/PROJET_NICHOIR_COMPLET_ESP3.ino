#include "M5TimerCAM.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_sleep.h>
#include "esp32/rtc.h"
#include "base64.h"
#include <WebServer.h>
#include <Preferences.h>
#include "driver/gpio.h"

// ================= GPIO ==================
const int SENSOR_PIN = 13;  // Capteur PIR connecté sur GPIO13
const int LED_PIN    = 4;   // LED utilisée comme indicateur de prise de photo
static const int GPIO33 = 33;
#define CAM_POWER_PIN 32   // GPIO alimentation caméra
// ================= TIMING ==================
// minute pour l'envoi du topic batterie
#define PERIOD_US (24ULL * 60ULL * 60ULL * 1000000ULL)   // Période d'envoi de l'état batterie 24heures
#define WINDOW_US (12ULL * 60ULL * 60ULL * 1000000ULL)   // 12h fenêtre temporelle maximale pendant laquelle 2 photos sont autorisées
#define PIR_BLOCK_US (5ULL * 60ULL * 1000000ULL)         // 5 minutes blocage après photo

// ================= WIFI ==================
Preferences preferences;
WebServer server(80);
// Page HTML de configuration Wi-Fi
String setupPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head><title>ESP32 Wi-Fi Setup</title></head>
<body>
<h2>Change Wi-Fi Credentials</h2>
<form action="/save" method="POST">
  SSID:<br>
  <input type="text" name="ssid"><br><br>
  Password:<br>
  <input type="password" name="password"><br><br>
  <input type="submit" value="Save">
</form>
</body>
</html>
)rawliteral";
// Page principale affichée lorsque l’ESP32 est connecté au Wi-Fi
String mainPageTemplate = R"rawliteral(
<!DOCTYPE html>
<html>
<head><title>ESP32 Wi-Fi Info</title></head>
<body>
<h2>ESP32 is connected to Wi-Fi</h2>
<p>SSID: %SSID%</p>
<p>IP: %IP%</p>
<p><a href="/setup">Change Wi-Fi</a></p>
</body>
</html>
)rawliteral";

// ================= MQTT ==================
#define MQTT_HOST "192.168.178.29"
#define MQTT_PORT 1883
#define MQTT_TOPIC_BATTERY "battery"
#define MQTT_TOPIC_PIC "pics"

// ================= RTC DATA ==================
// Timer batterie INDEPENDANT, en us, basé sur le RTC
// Prochain réveil programmé pour l’envoi batterie (timer RTC indépendant)
RTC_DATA_ATTR uint64_t next_battery_wakeup = 0;

// Début de la fenêtre PIR
RTC_DATA_ATTR uint64_t window_start      = 0;

// Nombre de déclenchements PIR dans la fenêtre courante
RTC_DATA_ATTR uint8_t  pir_count         = 0;

// Horodatage du dernier déclenchement PIR valide
RTC_DATA_ATTR uint64_t last_pir_trigger  = 0;

// Flag non utilisé actuellement 
RTC_DATA_ATTR bool pir_trigger_valid     = false;

// Indique si le PIR est temporairement bloqué après une photo
RTC_DATA_ATTR bool pir_block_active      = false;

// ================= MQTT OBJECTS ==================
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// ==================== WI-FI ====================
bool connectWiFiFromPrefs() {
    preferences.begin("wifi", true);
    String ssid = preferences.getString("ssid", "");
    String password = preferences.getString("password", "");
    preferences.end();

    if (ssid == "") return false;

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    for (int i = 0; i < 20; i++) {
        if (WiFi.status() == WL_CONNECTED) return true;
        delay(500);
    }
    return false;
}

void handleMain() {
    String page = mainPageTemplate;
    page.replace("%SSID%", WiFi.SSID());
    page.replace("%IP%", WiFi.localIP().toString());
    server.send(200, "text/html", page);
}

void handleSetup() {
    server.send(200, "text/html", setupPage);
}

void handleSave() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.end();

    server.send(200, "text/html", "<p>Saved! ESP32 will restart...</p>");
    delay(2000);
    ESP.restart();
}

// ==================== MQTT ====================
void connectMQTT() {
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    while (!mqttClient.connected()) {
        mqttClient.connect("TimerCAM_Client");
        delay(500);
    }
}

// ==================== PHOTO ====================
void sendPhotoInChunks() {
    if (!TimerCAM.Camera.get()) return;
    uint8_t* buf = TimerCAM.Camera.fb->buf;
    size_t len   = TimerCAM.Camera.fb->len;
    size_t chunkSize = 4095;
    int totalChunks = (len + chunkSize - 1) / chunkSize;
    String imageID = String(millis());

    for (int i = 0; i < totalChunks; i++) {
        size_t start = i * chunkSize;
        size_t sz = min(chunkSize, len - start);
        String base64Chunk = base64::encode(buf + start, sz);
        String payload = imageID + "|" + String(i) + "|" + String(totalChunks) + "|" + base64Chunk;
        mqttClient.publish(MQTT_TOPIC_PIC, payload.c_str());
        mqttClient.loop();
        delay(20);
    }
    TimerCAM.Camera.free();
}

void sendPhotoTakenNotification() {
    mqttClient.publish(MQTT_TOPIC_PIC, "Photo taken");
    digitalWrite(LED_PIN, LOW);
    Serial.println("Photo send");
    digitalWrite(CAM_POWER_PIN, LOW);
    unsigned long t0 = millis();
    while (millis() - t0 < 500) {
        mqttClient.loop();
        delay(10);
    }
}

// ======== NOUVELLE FONCTION POUR ENVOI BATTERIE ========
void sendBattery() {
    int batteryVoltage = TimerCAM.Power.getBatteryVoltage();
    int batteryLevel   = TimerCAM.Power.getBatteryLevel();
    char batteryMsg[40];
    sprintf(batteryMsg, "%d,%d", batteryLevel, batteryVoltage);

    if (connectWiFiFromPrefs()) {
        connectMQTT();
        mqttClient.publish(MQTT_TOPIC_BATTERY, batteryMsg);
        unsigned long t0 = millis();
        while (millis() - t0 < 700) { mqttClient.loop(); delay(20); }
        delay(300);
        WiFi.disconnect(true);
    }

    // Réinitialisation du timer batterie 24h après envoi avec photo
    next_battery_wakeup = esp_rtc_get_time_us() + PERIOD_US;
}

// ==================== SLEEP ====================
void goToSleep() {
    uint64_t now = esp_rtc_get_time_us();
   
    // Couper BT
    btStop();
    // Couper caméra
    digitalWrite(CAM_POWER_PIN, LOW);
    // 1) Initialisation du timer batterie si nécessaire
    if (next_battery_wakeup == 0) {
        // Alignement sur un multiple entier de PERIOD_US
        next_battery_wakeup = now - (now % PERIOD_US) + PERIOD_US;
    }

    // 2) Temps jusqu'au prochain envoi batterie (INDEPENDANT de la PIR)
    uint64_t t_bat;
    if (next_battery_wakeup > now) {
        t_bat = next_battery_wakeup - now;
    } else {
        t_bat = 0; // on est déjà en retard, on réveillera tout de suite pour envoyer
    }

    // 3) Temps jusqu'aux événements liés à la PIR (blocage / fin de fenêtre)
    uint64_t t_pir_timer = UINT64_MAX;

    if (pir_block_active) {
        uint64_t block_end = last_pir_trigger + PIR_BLOCK_US;
        if (block_end > now) {
            t_pir_timer = block_end - now;
        } else {
            t_pir_timer = 0;  // blocage déjà terminé
        }
    } else if (pir_count >= 2 && window_start != 0) {
        uint64_t window_end = window_start + WINDOW_US;
        if (window_end > now) {
            t_pir_timer = window_end - now;
        } else {
            t_pir_timer = 0;  // fenêtre déjà terminée
        }
    }

    // 4) On dort jusqu'au prochain événement : min(batterie, PIR)
    uint64_t sleep_time = t_bat;
    if (t_pir_timer < sleep_time) sleep_time = t_pir_timer;
    if (sleep_time == 0) sleep_time = 1000000ULL; // sécurité: au moins 1s

    // Timer toujours activé sert pour batterie + PIR timers
    esp_sleep_enable_timer_wakeup(sleep_time);

    // EXT0 (PIR) activé uniquement si pas de blocage et pas de fenêtre saturée
    bool pir_window_saturee = (pir_count >= 2 && window_start != 0);
    if (!pir_block_active && !pir_window_saturee) {
        esp_sleep_enable_ext0_wakeup((gpio_num_t)SENSOR_PIN, 1);
    }
       WiFi.disconnect(true);
    gpio_hold_en(GPIO_NUM_33);
    gpio_deep_sleep_hold_en();
    delay(200);
    esp_deep_sleep_start();

}

// ==================== SETUP ====================
void setup() {

    pinMode(CAM_POWER_PIN, OUTPUT);
    digitalWrite(CAM_POWER_PIN, LOW);
    // ===== POWER HOLD (CRITIQUE) =====
    gpio_set_direction(GPIO_NUM_33, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_33, 1);      // maintien alim
    gpio_hold_en(GPIO_NUM_33);           // fige l'état
    gpio_deep_sleep_hold_en();           // conserve en deep sleep
    // =================================
    
    Serial.begin(115200);
    delay(500);

    pinMode(SENSOR_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    TimerCAM.begin(true);
  

    uint64_t now = esp_rtc_get_time_us();
    esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();

// ================= Wi-Fi SETUP MODE =================
    if (!connectWiFiFromPrefs()) {
    Serial.println("[WiFi] No saved credentials, starting AP mode");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32-Setup", "12345678");
    Serial.print("[AP] Connect to: "); Serial.println("ESP32-Setup");
    Serial.print("[AP] Open browser to: "); Serial.println(WiFi.softAPIP());

    server.on("/", handleMain);
    server.on("/setup", handleSetup);
    server.on("/save", HTTP_POST, handleSave);
    server.begin();

    while (true) {
        server.handleClient();
        delay(10);
    }
}
    // ================= PIR WAKEUP =================
    if (reason == ESP_SLEEP_WAKEUP_EXT0) {
        Serial.println("Reveil par PIR");

        if (pir_block_active) goToSleep();

        if (window_start == 0 || (now - window_start) > WINDOW_US) {
            window_start = now;
            pir_count = 0;
        }

        if (pir_count < 2) {
            unsigned long windowStart = millis();
            int samples = 0, highCount = 0;
            while (millis() - windowStart < 10000) {
                samples++;
                if (digitalRead(SENSOR_PIN) == HIGH) highCount++;
                delay(50);
            }

            // Rejet si signal instable
            if ((float)highCount / samples < 0.7) goToSleep();

            pir_count++;
            //  Allumer caméra
            digitalWrite(CAM_POWER_PIN, HIGH);

            if (!TimerCAM.Camera.begin()) goToSleep();
             
             // Configuration caméra
            TimerCAM.Camera.sensor->set_pixformat(TimerCAM.Camera.sensor, PIXFORMAT_JPEG);
            TimerCAM.Camera.sensor->set_framesize(TimerCAM.Camera.sensor, FRAMESIZE_XGA);
            TimerCAM.Camera.sensor->set_vflip(TimerCAM.Camera.sensor, 1);
            TimerCAM.Camera.sensor->set_quality(TimerCAM.Camera.sensor, 3);

            digitalWrite(LED_PIN, HIGH);

            if (connectWiFiFromPrefs()) {
                connectMQTT();
                sendPhotoInChunks();
                sendPhotoTakenNotification();
                sendBattery();  // ENVOI BATTERIE SIMULTANÉ AVEC PHOTO ET RESET TIMER 24H
            }

            last_pir_trigger = now;
            pir_block_active = true;

            WiFi.disconnect(true);
            delay(300);
            
            Serial.println("Sleep");
            goToSleep();
        }

        goToSleep();
    }

    // ================= TIMER WAKEUP =================
    else if (reason == ESP_SLEEP_WAKEUP_TIMER) {
        Serial.println("Reveil par TIMER");

        // 1) Gestion des états PIR basés sur le temps (blocage, fenêtre)
        if (pir_block_active && now - last_pir_trigger >= PIR_BLOCK_US) {
            pir_block_active = false;
        }

        if (window_start != 0 && (now - window_start) >= WINDOW_US) {
            window_start = now;
            pir_count = 0;
        }

        // 2) Gestion du timer batterie INDEPENDANT
        if (next_battery_wakeup == 0) {
            next_battery_wakeup = now - (now % PERIOD_US) + PERIOD_US;
        }

        bool doit_envoyer_batterie = (now >= next_battery_wakeup);

        if (doit_envoyer_batterie) {
            int batteryVoltage = TimerCAM.Power.getBatteryVoltage();
            int batteryLevel   = TimerCAM.Power.getBatteryLevel();
            char batteryMsg[40];
            sprintf(batteryMsg, "%d,%d", batteryLevel, batteryVoltage);

            if (connectWiFiFromPrefs()) {
                connectMQTT();
                mqttClient.publish(MQTT_TOPIC_BATTERY, batteryMsg);
                unsigned long t0 = millis();
                while (millis() - t0 < 700) { mqttClient.loop(); delay(20); }
                delay(300);
                WiFi.disconnect(true);
            }

             // Avance sans dérive du timer batterie
            while (next_battery_wakeup <= now) {
                next_battery_wakeup += PERIOD_US;
            }
        }

        goToSleep();
    }

    // ================= FIRST BOOT =================
    else {
        // Initialisation propre au tout premier démarrage
        window_start      = now;
        pir_count         = 0;
        pir_block_active  = false;
        pir_trigger_valid = false;

        next_battery_wakeup = now - (now % PERIOD_US) + PERIOD_US;

        goToSleep();
    }
}

void loop() {
    // Si AP mode actif, gérer le serveur
    if (WiFi.getMode() == WIFI_AP) {
        server.handleClient();
    }
}
