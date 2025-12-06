# ESP32 Communication par MQTT

## Introduction
Ce dossier contient les codes liés à la communication MQTT du module ESP32 5TimerCam. Les codes permettent d'envoyer au broker MQTT des topics contenant des données textuelles ainsi que des images.  
Les codes sont dévelloppés sur l'environnement Arduino IDE et incluent la gestion de la connexion internet du côté subscriber. 
L'échange de données s'effectue sur le réseau local, via l'adresse IP du broker MQTT.

---

## Patch 7#
**Objectif :** Envoi d'une photo MQTT après détection de mouvement prolongée.  
Le but est de détecter un mouvement via un capteur PIR, attendre que ce mouvement dure au moins 10 secondes, capturer une photo avec la TimerCAM et l’envoyer vers un broker MQTT en morceaux (chunks) encodés en Base64.

- Configuration matérielle
  - Capteur PIR connecté sur la pin 13
  - Lecture en numérique : HIGH = mouvement détecté
  - TimerCAM initialisée en mode caméra JPEG
  - Résolution configurée en XGA 
  - Orientation verticale inversée via set_vflip(1)

- Connexion WiFi
  - Connexion au réseau WiFi défini dans les macros WIFI_SSID et WIFI_PASSWORD
  - Timeout de connexion fixé à 10 secondes 
  - Reconnexion automatique activée
  - Affichage de l’adresse IP en cas de réussite

- Communication MQTT
  - Utilisation de la librairie PubSubClient
  - Connexion au broker MQTT défini par :
    - MQTT_HOST
    - MQTT_PORT
    - MQTT_TOPIC
  - Reconnexion automatique si la communication MQTT est perdue
  - Envoi des messages via mqttClient.publish

- Détection de mouvement
  - La logique du PIR fonctionne en trois phases :
    1 - Détection du front montant
        - Passage LOW à HIGH
        - Début de la temporisation : motionStart = millis()
        - Le flag photoTaken = false réactive la prise de photo

    2 - Maintien du mouvement
        - On attend 10 secondes de mouvement continu
        - Condition : millis() - motionStart >= 10000
        - Une fois la durée atteinte → capture d'une photo

    3 - Fin du mouvement
        - Passage HIGH à LOW
        - Reset de l’état du capteur

- Capture et envoi de la photo
  - Prise de vue via TimerCAM en JPEG
  - Découpage de l’image en blocs de 4 KB
  - Encodage en Base64 pour compatibilité MQTT
  - Transmission séquentielle de tous les chunks
  - Libération de la mémoire caméra


## Patch 8#
**Objectif :** Envoi d'une photo MQTT après détection de mouvement prolongée avec une mise en veille.  
Détecter un mouvement continu durant 10 secondes, activer le WiFi, envoyer une photo via MQTT, puis repasser l’ESP32 TimerCAM en mode veille profonde avec réveil par le capteur PIR.

- Configuration matérielle
  - Capteur PIR sur GPIO13
  - TimerCAM initialisée en mode JPEG
  - Résolution XGA
  - Orientation verticale ajustée et qualité optimisée
  - Port série actif pour le debug

- Détection de mouvement 

  - Le PIR pilote l’ensemble du fonctionnement à travers trois phases :

    1 - Début du mouvement

        - Passage de LOW à HIGH.
        - Enregistrement du temps de début.
        - La prise de photo n’est pas encore autorisée.

    2 - Mouvement continu pendant 10 secondes

        - Si le PIR reste HIGH pendant 10 secondes il y a un déclenchement de la capture.
        - Le système sait que le mouvement est réel et persistant.

    3 - Fin du mouvement

        - Retour à LOW ainsi le retour à l’état initial.

- Connexion WiFi

  - Activation du WiFi uniquement lorsque la photo doit être envoyée.
  - Connexion au point d’accès défini dans le code.
  - Affichage de l’adresse IP pour vérifier la liaison.
  - Extinction complète du WiFi après envoi pour économiser l’énergie.

- Communication MQTT

  - Configuration du broker: L'adresse IP, port et topic.
  - Connexion MQTT juste avant l’envoi de la photo.
  - Reconnexion si nécessaire.
  - Transmission d’un message texte “Photo taken” après l’image.

- Capture et envoi de la photo

  - Capture JPEG via TimerCAM.
  - Découpage en blocs de 4 KB.
  - Encodage Base64 pour compatibilité MQTT.
  - Envoi séquentiel de chaque chunk identifié par:
    - ID unique: timestamp.
    - Numéro du chunk.
    - Nombre total de chunks.
  - Libération du buffer caméra après transmission.

- Mise en veille après envoi
  
  - Extinction complète du WiFi: disconnect et WIFI_OFF.
  - Attente du retour du PIR à LOW jusqu’à 10 secondes.
  - Configuration du réveil via ext0_wakeup sur le PIR.
  - Passage en deep sleep pendant 20 secondes via TimerCAM si aucune photo est prise.
    - Cela permet :
      - Réduction très forte de la consommation énergétique.
      - Réveil automatique dès qu’un nouveau mouvement est détecté.


## Patch 9#
**Objectif :** .  

- 
