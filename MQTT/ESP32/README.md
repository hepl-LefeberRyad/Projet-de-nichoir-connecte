# ESP32 Communication par MQTT

## Introduction
Ce dossier contient les codes liés à la communication MQTT du module ESP32 5TimerCam. Les codes permettent d'envoyer au broker MQTT des topics contenant des données textuelles ainsi que des images.  
Les codes sont dévelloppés sur l'environnement Arduino IDE et incluent la gestion de la connexion internet du côté subscriber. 
L'échange de données s'effectue sur le réseau local, via l'adresse IP du broker MQTT.

---
## Patch 4#  

**Objectif** 
Ce code met en place un système de double réveil pour la TimerCAM :
- Réveil immédiat via le capteur PIR pour signaler un mouvement
- Réveil périodique via le timer RTC toutes les 60 secondes
Le tout en maintenant une consommation minimale grâce au deep sleep et à un timer permanent synchronisé.

1. Configuration matérielle
- LED externe sur le GPIO4
- Capteur PIR sur le GPIO13
- TimerCAM initialisée au démarrage
- RTC interne utilisé pour mesurer le temps et programmer les réveils

2. Gestion des causes de réveil
Au démarrage, le code identifie la source du réveil :

a) Réveil par PIR (EXT0)
- Le PIR est passé à HIGH
- La LED externe s’allume pendant 10 secondes
- Puis elle s’éteint avant le retour en deep sleep
  Ce comportement simule une action simple lors d’un mouvement détecté.

b) Réveil par Timer RTC
- Le timer interne a atteint l’échéance programmée
- Le code affiche simplement : "Reveil par timer permanent"
  Ce réveil sert uniquement à maintenir la synchronisation du timer.

c) Premier démarrage / reset
- Initialisation du timer permanent
- next_wakeup = now + PERIOD_US
  Le premier réveil périodique est programmé exactement 60 secondes plus tard.

3. Timer RTC permanent 
Le code utilise une variable RTC persistante :
RTC_DATA_ATTR uint64_t next_wakeup;


Elle survit au deep sleep et permet de :
- Garder une échéance fixe dans le temps
- Éviter toute dérive, même après plusieurs réveils PIR
- Recalculer la durée restante avant le prochain réveil
Logique :
- On lit l’heure RTC 
- On calcule le temps restant
- Ce temps restant est utilisé pour programmer le prochain réveil timer

Le réveil périodique reste parfaitement aligné toutes les 60 secondes.

4. Réactivation des sources de réveil
Avant chaque deep sleep :
- Le capteur PIR est activé :
esp_sleep_enable_ext0_wakeup(SENSOR_PIN, 1);
- Timer RTC est activé avec la durée restante :
esp_sleep_enable_timer_wakeup;
Ainsi, l’ESP32 peut être réveillé :
- Instantanément par un mouvement
- Régulièrement par le timer interne
- Sans conflit entre les deux mécanismes

5. Deep sleep
Enfin, le module entre en veille profonde:

esp_deep_sleep_start();

Le loop() n’est jamais exécuté : tout se passe dans setup() à chaque réveil.

---

## Patch 5# 
**Objectif** 

Ce firmware combine deux mécanismes de réveil pour la TimerCAM :
     • 	Réveil PIR pour allumer une LED lors d’un mouvement
     • 	Réveil périodique RTC toutes les 60 secondes
Le tout avec une fenêtre PIR de 10 minutes limitant le nombre d’activations à 2, et un timer permanent synchronisé pour éviter toute dérive temporelle.

1. Configuration matérielle
     • 	LED externe sur le pin GPIO4
     • 	Capteur PIR sur le pin GPIO13 
     • 	TimerCAM initialisée au démarrage
     • 	RTC interne utilisé pour mesurer le temps et gérer les fenêtres

2. Gestion du réveil PIR
Lors d’un réveil par le capteur PIR :
    • 	Vérification de la fenêtre PIR de 10 minutes :
    • 	Si première activation ou fenêtre expirée: remise à zéro, déboute une nouvelle   fenêtre
    • 	Limitation à 2 déclenchements de la LED.
    • 	La LED s'allumée pendant 10 secondes
    • 	Sinon la LED est ignorée quand la limite est atteinte

Ce mécanisme évite les déclenchements répétés et stabilise la consommation.

3. Réveil par Timer RTC
Si le réveil provient du timer interne :
    • 	Le code affiche simplement :"Reveil par timer permanent"
Ce réveil n’exécute aucune action fonctionnelle :
il sert uniquement à maintenir la synchronisation du timer permanent.

4. Premier démarrage
Lors d’un reset ou du tout premier boot :
    • 	Initialisation du timer permanent :

    • 	Initialisation de la fenêtre PIR
    • 	Remise à zéro du compteur PIR

5. Timer RTC permanent 
Le code utilise une variable RTC persistante :

Elle permet de :
    • 	Garder une échéance fixe toutes les 60 secondes
    • 	Recalculer le temps restant avant le prochain réveil
    • 	Éviter toute dérive, même après plusieurs réveils PIR
Logique :
    • 	Si l’heure actuelle dépasse l’échéance alors on programme la suivante
    • 	Sinon, on calcule le temps restant
    • 	Ce temps est utilisé pour programmer le réveil RTC

6. Activation des sources de réveil
Avant chaque deep sleep :
    • 	Le capteur PIR activé :

    • 	Timer RTC activé avec la durée restante :

Ainsi, l’ESP32 peut être réveillé :
    • 	Instantanément par le PIR
    • 	Régulièrement par le timer RTC
    • 	Sans conflit entre les deux mécanismes

---


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
    
---

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
    - Début du mouvement
      - Passage de LOW à HIGH.
      - Enregistrement du temps de début.
      - La prise de photo n’est pas encore autorisée.

    - Mouvement continu pendant 10 secondes
      - Si le PIR reste HIGH pendant 10 secondes il y a un déclenchement de la capture.
      - Le système sait que le mouvement est réel et persistant.

    - Fin du mouvement
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
---

## Patch 9#
**Objectif :** Envoi d'une photo MQTT avec flash après détection de mouvement prolongée avec une mise en veille .  
Détecter un mouvement continu durant 10 secondes, signaler la prise de photo via une LED externe, envoyer l’image en plusieurs blocs sur MQTT, puis repasser l’ESP32 TimerCAM en mode veille profonde avec réveil via le PIR.

- Configuration matérielle
  - Capteur PIR connecté sur GPIO13
  - LED externe sur GPIO4
  - TimerCAM initialisée pour capturer en JPEG 
  - Port série configuré pour le suivi en temps réel
  - Paramètres MQTT définis: broker local, topic

- Détection du mouvement
  - Le programme suit l’évolution du signal du PIR :
  - Début du mouvement: LOW à HIGH
  - Enregistrement du temps de début
  - Indication dans le terminal
  - L’appareil se prépare mais ne prend pas encore de photo

- Mouvement continu pendant 10 secondes
  - Vérification via millis()
  - Ce seuil valide que la présence détectée est réelle et prolongée

- Fin du mouvement 
  - Réinitialisation du système
  - Retour en état d’attente

- Indication lumineuse
  - La LED s’allume juste avant la capture photo
  - Elle reste active pendant la transmission MQTT
  - Elle s’éteint 2 secondes après la photo, comme flash

- Connexion WiFi et communication MQTT
  - Activation du WiFi uniquement lors de la prise de photo
  - Connexion au réseau local configuré
  - Connexion MQTT au broker via un client PubSub
  - Envoi d’un message “Photo taken” après le transfert de l’image
  - Désactivation totale du WiFi après transmission

- Capture et envoi de la photo
  - Capture JPEG via TimerCAM
  - Découpage du fichier en blocs de 4 KB
  - Encodage Base64 de chaque bloc
  - Envoi sur le topic MQTT sous forme de paquets :

    - ID de l’image
      - Numéro du bloc
      - Nombre total de blocs
      - Données base64

  - Libération du buffer caméra après transmission

- Mise en veille et gestion du réveil
  - Attente que le PIR repasse à LOW avant d’autoriser la veille
  - Configuration du réveil matériel via ext0_wakeup sur le capteur PIR
  - Passage en mode deep sleep pendant 20 secondes si aucune photo est prise
  - L’ESP32 se réveillera automatiquement lorsqu’un nouveau mouvement est détecté

    - Ce fonctionnement permet :
      - Une détection fiable
      - Une transmission réseau efficace
      - Une consommation d’énergie minimale grâce à la veille profonde

---

## Patch 10# 
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
   - pir_count (compteur de photos max. 2 photos dans la fenêtre)

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

---

## Patch 11#
**Objectif** L’objectif est de faire fonctionner la TimerCAM en ultra basse consommation en combinant trois fonctions totalement indépendantes : la prise de photo uniquement lorsqu’un mouvement est validé par le capteur PIR, l’envoi périodique de l’état de la batterie via MQTT, et un mode deep sleep optimisé utilisant deux sources de réveil — le PIR pour les photos et le timer RTC pour les transmissions programmées.

1. Matériel et configuration
   
- Capteur PIR sur le pin GPIO13 (EXT0)
- LED externe sur le pin GPIO4
- Initialisation de la TimerCAM
- Initialisation du Wi-Fi
- Initialisation de la communication par MQTT 

2. Réveil par détection d'un mouvement par le capteur PIR  
 Au réveil par le PIR :

 - Vérification du contexte s'il n'y a pas de blocage ou si la fenêtre PIR active
 - Validation du mouvement :
  - Observation du PIR pendant 10 s
  - Le signal doit rester majoritairement HIGH
  - Sinon: retour en deep sleep
 - Limitation : 2 photos maximum par fenêtre de 12h  
  - window_start = début de fenêtre
  - pir_count = nombre de photos prises

3. Capture & envoi photo
   
Si le mouvement est validé :
- Initialisation caméra
  - Capture d'une image
  - Découpage en blocs de 4095 octets
  - Encodage Base64
  - Envoi MQTT des chunks de la photo
  - Message final : "Photo taken"
  - Libération mémoire + coupure Wi-Fi
- Activation d’un blocage PIR de 5 minutes

4. Blocage PIR
   
Après chaque photo :
- Le PIR est ignoré pendant 5 minutes
- Le timer RTC gère automatiquement la fin du blocage
- Objectif : éviter les rafales et réduire la consommation

5. Batterie réveil par TIMER
   
Un réveil RTC indépendant envoie la batterie toutes les 24 h :
- Lecture tension + le pourcentage de la batterie
- Publication MQTT sur battery
- Coupure Wi-Fi
- Retour en deep sleep
Ce réveil fonctionne même si plusieurs réveils PIR ont eu lieu entre-temps.

6. Gestion des timers RTC
   
- Timer batterie :
- Stocké en RTC
- Toujours aligné sur un multiple exact de 24 h
- Fenêtre PIR :
- Durée : 12 h
- À expiration alors pir_count = 0
- Avant chaque deep sleep :
- Calcul du prochain événement :
- Envoi batterie
- Fin blocage capteur PIR
- Fin fenêtre capteur PIR
- Le plus proche détermine la durée du sommeil

7. Mise en veille
   
Avant de dormir :
- Timer RTC toujours actif
- PIR activé uniquement si : Pas de blocage ou si la fenêtre PIR n'est pas saturée.
- Fenêtre PIR non saturée

Réveil après dormir si :
- Réveil instantané sur mouvement valide
- Batterie envoye les données régulièrement
- Ce qui permet une consommation minimale

