## Patch hepl-SousaJoaopatch-10# 
**Objectif :** Détecter un mouvement continu pendant 10 secondes, capturer une photo, l’envoyer en blocs via MQTT, puis remettre la TimerCAM en deep sleep avec double réveil :
   - Par capteur PIR (EXT0) pour prendre les photos
   - Par timer RTC pour l’envoi périodique du niveau de batterie.
 Ce patch combine détection fiable, transmission optimisée, et gestion d’énergie avancée.

1. Configuration matérielle
   - Capteur PIR sur le GPIO13
   - LED externe sur GPIO4
   - TimerCAM initialisée pour capturer en JPEG 
   - WiFi activé uniquement lors des transmissions
   - MQTT : broker local, topics pics et timercam/battery

2. Détection du mouvement avec la PIR:

   - Vérification que le PIR est HIGH 
   - Démarrage d’une fenêtre de 10 secondes via millis()
   - Si le signal reste stable : photo autorisée
   - Sinon : retour immédiat en veille
  Une limite de 2 photos par fenêtre de 10 minutes est imposée via :
   - window_start (timestamp RTC)
   - pir_count => compteur de photos max. 2 photos par fenêtre de temps

3. Capture et envoi de la photo

 Si le mouvement est continu pendant 10 secondes:
   - Initialisation caméra
   - Capture JPEG
   - Découpage en blocs de 4095 bytes
   - Encodage Base64
   - Envoi MQTT sous forme de données_base64
   - Message final : "Photo taken"
   - Libération du buffer caméra
   - Désactivation du WiFi

4. Envoi périodique de la batterie (réveil par le TIMER)
 Toutes les 1 minute, un réveil RTC est déclenché :
   - Lecture tension + pourcentage batterie
   - Connexion WiFi + MQTT
   - Publication sur timercam/battery
   - Retour en deep sleep
 Ce réveil n’interfère pas avec la logique PIR.

5. Gestion des timers RTC 
 a) Réveil toutes les 60 secondes pour envoyer l’état batterie.
    #define PERIOD_US (60ULL * 1000000ULL)

 b) WINDOW_US – Fenêtre PIR de 10 minutes
    Limite de 2 photos par fenêtre de 10 minutes.
    Si la fenêtre expire, pir_count est remis à zéro. 
    #define WINDOW_US (10ULL * 60ULL * 1000000ULL)

 c) next_wakeup – Synchronisation continue
    Stocké en RTC_DATA_ATTR, donc conservé en deep sleep.
    Logique :
   - On calcule le temps restant avant le prochain réveil périodique
   - On programme le timer RTC avec ce temps
   - On met à jour next_wakeup = now + remaining

 Le réveil batterie reste parfaitement synchronisé, même si plusieurs réveils PIR interviennent entre-temps.

  6. Mise en veille et réveils
   
  Avant de dormir :
   - Activation du réveil PIR :
     esp_sleep_enable_ext0_wakeup(SENSOR_PIN, 1)
   - Activation du réveil timer :
     esp_sleep_enable_timer_wakeup(remaining)
   - Passage en deep sleep
 Ce fonctionnement assure :
   - Réactivité avec un réveil instantané sur mouvement
   - Régularité avec le niveau de batterie envoyée toutes les 60 s
   - Économie d’énergie maximale


