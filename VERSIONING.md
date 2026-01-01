# Versioning

Chaque changement apporté aux codes a été réalisé en utilisant une nouvelle branche.  
La branche `main` contient le code principal avec l’ensemble des fonctionnalités du projet.

Dans chaque branche créée, un fichier README décrit les changements apportés aux différents fichiers du code.  
Dans ce README, nous rassemblons l’ensemble des différents patchs appliqués à notre projet afin d’assurer un suivi clair et structuré de son évolution.

## Patchs effectuer par Ryad
---

## Patch #1
**Objectif :** Gestion des messages envoyés par l'ESP32 via MQTT.  
- Création de la base `IMAGE` et de la table `messages`.  
- Champs principaux de la table `messages` :
  - `id` : identifiant unique auto-incrémenté.  
  - `topic` : topic MQTT associé au message.  
  - `payload` : contenu du message (texte).  
  - `timestamp` : date et heure de réception automatique.  
- Script Python fourni pour :
  - Créer la table si elle n’existe pas (`create_table()`).  
  - Recevoir des messages MQTT et les insérer dans la base (`insert_message()`).  
- Utilisation d’un client MQTT en Python (`paho-mqtt`) avec gestion de la reconnexion automatique.

---

## Patch #2
**Objectif :** Réception et stockage des images envoyées en un seul paquet.  
- Création de la table `images` pour stocker les photos.  
- Champs principaux :
  - `id` : identifiant unique auto-incrémenté.  
  - `topic` : topic MQTT associé à l'image.  
  - `payload` : données binaires de l'image (BLOB).  
  - `timestamp` : date et heure de réception.  
- Script Python fourni pour :
  - Décoder les images envoyées en Base64.  
  - Sauvegarder les images :
    - Dans le dossier local `/home/joao1/received_images`.  
    - Dans la base de données MariaDB (`images`).  
- Gestion des erreurs de connexion et de décodage des images.

---

## Patch #3
**Objectif :** Gestion des images envoyées en plusieurs morceaux (chunking).  
- Réception des images chunkées pour permettre l’envoi de fichiers plus volumineux.  
- Structure temporaire : dictionnaire `image_chunks` pour stocker les morceaux d’images en mémoire.  
- Fonctionnalités principales :
  - Assemblage des chunks dans le bon ordre pour reconstituer l'image complète.  
  - Décodage Base64 de l’image reconstituée.  
  - Sauvegarde automatique :
    - Dans le dossier local `/home/joao1/received_images`.  
    - Dans la base de données MariaDB (`images`) avec `imageID`.  
  - Gestion de la mémoire pour supprimer les chunks après assemblage.  
  - Logs détaillés pour suivre la réception et le traitement des images.

--- 
---

### Patch 4 – Ajout du serveur Flask pour récupérer les messages MQTT
- **Fichier modifié :** `app.py`
- **Ajouts :**
  - Création d'un serveur Flask minimal.
  - Connexion à MariaDB pour récupérer les messages MQTT.
  - Route `/` qui affiche les messages dans une page HTML via `render_template`.
  - Fonction `get_messages()` pour interroger la table `messages`.
  - Serveur prêt à afficher les messages côté web.

---

### Patch 5 – Ajout de la page HTML pour l’affichage des messages
- **Fichier ajouté :** `index.html`
- **Ajouts :**
  - Interface web pour afficher les messages MQTT.
  - Tableau avec colonnes : `ID`, `Topic`, `Payload`, `Timestamp`.
  - Style CSS simple pour lisibilité et centrage du contenu.
  - Affichage dynamique des messages récupérés par Flask.

---

### Patch 6 – Ajout de la gestion des images dans Flask
- **Fichier modifié :** `app.py`
- **Ajouts :**
  - Gestion des images reçues via MQTT (Base64 pour le front).
  - Fonction `get_images()` pour récupérer les images depuis MariaDB.
  - Passage de la liste des images à la page HTML pour affichage.
  - Préparation du code pour afficher les images dans une interface web dédiée.

---

### Patch 7 – Page HTML pour afficher les images
- **Fichier ajouté :** `index.html`
- **Ajouts :**
  - Interface web pour visualiser les images envoyées via MQTT.
  - Chaque image affichée dans une carte avec ses informations : `id`, `topic`, `timestamp`.
  - Modal viewer pour agrandir les images au clic.
  - Auto-refresh toutes les 5 secondes pour mettre à jour les images côté front.

---

### Patch 8 – Ajout de la gestion de la batterie dans Flask
- **Fichier modifié :** `app.py`
- **Ajouts :**
  - Ajout d’un client MQTT pour recevoir les données de batterie (`level, voltage`).
  - Stockage temporaire des informations de batterie côté serveur (`battery_info`).
  - Les images sont toujours récupérées et envoyées au front.
  - Route `/` renvoie maintenant les images **et** les informations de batterie pour l’affichage.

---

### Patch 9 – Page HTML pour afficher images et batterie
- **Fichier modifié :** `index.html`
- **Ajouts :**
  - Bloc d’information sur la batterie en haut de la page (`Voltage`, `Capacity`, `Updated`).
  - Auto-refresh toutes les 5 secondes pour mettre à jour les images **et** les informations de batterie.
  - Modal viewer pour images maintenu pour l’interaction utilisateur.
  - Page complète affichant à la fois les images et l’état de la batterie.
--- 
## Patch #10
**Objectif :** Gestion des images envoyées en plusieurs morceaux (chunking) et ajout de la réception des informations de batterie via MQTT.

### Nouveautés
- **Gestion du topic batterie (`battery`)**
  - Prise en charge des messages MQTT envoyés par l'ESP32 contenant l’état de la batterie.
  - Format attendu : `"LEVEL,VOLTAGE"` (exemple : `"85,3700"`).
  - Extraction automatique de :
    - `battery_level` : niveau de batterie en pourcentage.
    - `battery_voltage` : tension en millivolts.
  - Affichage des informations dans les logs avec horodatage.
  - Gestion des erreurs pour les messages mal formés ou inattendus.

- **Adaptation des callbacks MQTT**
  - Les callbacks gèrent désormais simultanément :
    - Le topic images (`pics`) avec réception chunkée.
    - Le topic batterie (`battery`) sans perturber le traitement des images.

- **Suivi temps réel de l’état énergétique**
  - Permet de suivre l’état de la batterie de l’ESP32 via MQTT en parallèle de la réception et de la sauvegarde des images.
--- 

## Patchs effectuer par Joao
---
## Patch hepl-SousaJoao-patch-1#
**Objectif :** Test simple du capteur BS-612 qui contrôle l'état d'une led.  

- Configuration des pins du matérielle
  - Le capteur PIR BS-612 est configuré comme une entrée sur le GPIO13.
  - La led est configuré comme sortie sur le pin GPIO4.
- Lecture de l'état du capteur
  - Il y a une surveillance continue de l'état logique du capteur PIR.
  - Lorsqu'il y a une détection d'un front montant, le capteur se met en état HIGH.
- Activation de la LED
  - La led s'allume lors de la détection
  - Grâce au compteur millis(), le programme peut vérifier si la led est allumée depuis 5000ms.
- Gestion de la temporisation
  - Comptage jusqu'à 5000ms avant que la led s'éteint.
  - Utilisation d'un flag (ledActive) pour éviter des déclenchements répétés.
- Coupure automatique de la led
  - La led s'éteint.   



## Patch hepl-SousaJoao-patch-2#
**Objectif :** Détecter un mouvement avec le capteur PIR BS-612 et allumer la led uniquement si le mouvement dure au moins 5 secondes.  

- Configuration des pins du matérielle
  - Le capteur PIR BS-612 est configuré  sur le GPIO13 et la led sur le pin GPIO4.
  - L'initialisation de la led à l'état LOW.
- Lecture de l'état du capteur
  - Il y a une surveillance continue de l'état logique du capteur PIR.
  - Lorsqu'il y a une détection d'un front montant, le capteur se met en état HIGH.
- Démarrage du chronomètre sur détection
  - Quand un mouvement est capté, pour la première fois:
    - motionDetected = true
    - motionStartTime = millis(), il y a un enregistrement du moment du début du mouvement.
- Vérification de la durée du mouvement
  - Si le mouvement continue est égale ou dépasse 5 secondes alors:
    - La led s'allume.
    - Un message est envoyé sur le terminal:
- Réinitialisation si pas de mouvement
  - motionDeected = false
  - si la led est allumée, et il n'y a plus de mouvement continue: la led s'éteint.

         

## Patch hepl-SousaJoao-patch-3#
**Objectif :** Réveil du microcôntroleur grâce au capteur PIR BS-612.  

- Configuration des pins du matérielle
  - Le capteur PIR BS-612 est configuré  sur le GPIO13 et la led sur le pin GPIO4.
  - L'initialisation de la led à l'état LOW.
- Réveil par bouton ou capteur
  - Autorisation du réveil depuis l’état de sommeil profond (esp_sleep_enable_ext0_wakeup)
  - Niveau bas sur le bouton ou le capteur déclenche le réveil
  - Permet de garder l’appareil en veille pour économiser de l’énergie
- Gestion du TimerCAM
  - Initialisation du module TimerCAM (TimerCAM.begin(true))
  - Commande d’allumage de la LED via PWM 
  - LED allumée pendant 10 secondes pour indiquer le réveil

- Mise en veille automatique
  - Après affichage / temporisation, l’appareil peut se mettre en veille
  - Le réveil peut être déclenché par :
  - le capteur PIR
  - le bouton physique
  - La fonction TimerCAM.Power.timerSleep(5) déclenche la mise en veille après 5 secondes

- Utilisation du compteur / temporisation
  - vTaskDelay(pdMS_TO_TICKS(10000)) = pause non bloquante pendant 10 secondes avant de lancer la veille
  - Permet d’avoir un délai avant la mise en veille tout en restant compatible avec le multitâche

---
## Patch hepl-SousaJoao-patch-4#  

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

## Patch hepl-SousaJoao-patch-5# 
**Objectif** 

Ce firmware combine deux mécanismes de réveil pour la TimerCAM :
     - Réveil PIR pour allumer une LED lors d’un mouvement
     - Réveil périodique RTC toutes les 60 secondes
Le tout avec une fenêtre PIR de 10 minutes limitant le nombre d’activations à 2, et un timer permanent synchronisé pour éviter toute dérive temporelle.

1. Configuration matérielle
     - LED externe sur le pin GPIO4
     - Capteur PIR sur le pin GPIO13 
     - TimerCAM initialisée au démarrage
     - RTC interne utilisé pour mesurer le temps et gérer les fenêtres

2. Gestion du réveil PIR
Lors d’un réveil par le capteur PIR :
     - Vérification de la fenêtre PIR de 10 minutes :
     - Si première activation ou fenêtre expirée: remise à zéro, déboute une nouvelle fenêtre
     - Limitation à 2 déclenchements de la LED.
     - La LED s'allumée pendant 10 secondes
     - Sinon la LED est ignorée quand la limite est atteinte

Ce mécanisme évite les déclenchements répétés et stabilise la consommation.

3. Réveil par Timer RTC
Si le réveil provient du timer interne :
     - Le code affiche simplement :"Reveil par timer permanent"
Ce réveil n’exécute aucune action fonctionnelle :
il sert uniquement à maintenir la synchronisation du timer permanent.

4. Premier démarrage
 Lors d’un reset ou du tout premier boot :
   - Initialisation du timer permanent :
     - Initialisation de la fenêtre PIR
     - Remise à zéro du compteur PIR

5. Timer RTC permanent 
 Le code utilise une variable RTC persistante :

 Elle permet de :
  - Garder une échéance fixe toutes les 60 secondes
  - Recalculer le temps restant avant le prochain réveil
  - Éviter toute dérive, même après plusieurs réveils PIR
 
 Logique :
  -  Si l’heure actuelle dépasse l’échéance alors on programme la suivante
  -  Sinon, on calcule le temps restant
  -  Ce temps est utilisé pour programmer le réveil RTC

6. Activation des sources de réveil
 Avant chaque deep sleep :
  - Le capteur PIR activé
  - Timer RTC est activé avec la durée restante

Ainsi, l’ESP32 peut être réveillé :

  - Instantanément par le PIR
  - Régulièrement par le timer RTC
  - Sans conflit entre les deux mécanismes

---

## Patch hepl-SousaJoao-patch-6#  
**Objectif** 
Gestion PIR limitée, envoi batterie MQTT et timer RTC synchronisé.
Ce code met en ouvre un fonctionnement basse consommation combinant trois mécanismes indépendants:
- Déclenchement par le capteur PIR limité à deux activations dans une fenêtre de dix minutes.
- Envoi périodique de l’état de la batterie via MQTT lors d’un réveil par timer RTC.
- Deep sleep systématique avec double source de réveil : capteur PIR (EXT0) et timer RTC.
L’ensemble garantit une consommation minimale tout en assurant un comportement prévisible et synchronisé.

1. Configuration matérielle et réseau
- LED sur GPIO4
- PIR sur GPIO13 (réveil EXT0)
- TimerCAM initialisée au démarrage
- Wi‑Fi activé uniquement lors des transmissions MQTT
- MQTT configuré avec un broker local et un topic batterie

2. Gestion du réveil PIR
Lors d’un réveil PIR :
- Vérification de la fenêtre de dix minutes ; réinitialisation si expirée
- Si pir_count < 2, l’activation est acceptée : incrément du compteur et LED allumée dix secondes
- Si la limite est atteinte, aucune action n’est effectuée
Ce mécanisme limite les déclenchements et réduit la consommation.

3. Réveil par timer RTC et envoi de la batterie
Lors d’un réveil TIMER :
- Lecture du niveau et de la tension de batterie
- Construction d’un message unique
- Activation du Wi‑Fi, connexion MQTT, publication sur timercam/battery
- Désactivation immédiate du Wi‑Fi
Le Wi‑Fi n’est utilisé que pour l’envoi, ce qui optimise l’énergie.

4. Premier démarrage
En cas de reset :
- Initialisation du timer permanent, prochaine échéance dans une minute
- Initialisation de la fenêtre PIR
- Remise à zéro du compteur PIR

5. Timer RTC permanent et synchronisation
Le timer utilise next_wakeup stocké en RTC pour maintenir une périodicité stable :
- Si l’échéance est dépassée, une nouvelle est programmée
- Sinon, le temps restant est calculé
- Le timer RTC est configuré avec cette durée
Ce procédé évite toute dérive, même après plusieurs réveils PIR.

6. Activation des sources de réveil
Avant chaque deep sleep :
- Activation du réveil PIR (EXT0)
- Activation du réveil TIMER avec la durée restante
L’ESP32 peut ainsi être réveillé soit par un mouvement, soit par le timer périodique.

---    

## Patch hepl-SousaJoao-patch-7#
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
         
       - Une fois la durée atteinte une photo est capturée

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

## Patch hepl-SousaJoao-patch-8#
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

## Patch hepl-SousaJoaopatch-9#
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

---

## Patch hepl-SousaJoaopatch-11#
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

---

## Patch hepl-SousaJoaopatch-12#

**Objectif** Faire fonctionner la TimerCAM en ultra basse consommation : capture photo uniquement sur mouvement validé par le PIR, envoi périodique de la batterie via MQTT, et deep sleep optimisé avec GPIO33 pour alimentation de la caméra, capteur PIR et les timers RTC.

1. Matériel et configuration

   
- Capteur PIR sur le pin GPIO13 (EXT0)
- LED externe sur le pin GPIO4
- Initialisation du  GPIO33 poour commuter la batterie
- Initialisation de la TimerCAM
- Initialisation du Wi-Fi
- Initialisation de la communication par MQTT 
- GPIO 33 est mis sur HIGH en permanence

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
  - Capture d'une image + envois de la topic de la batterie
  - Découpage en blocs de 4095 octets
  - Encodage Base64
  - Envoi MQTT des chunks de la photo
  - Message final : "Photo taken"
  - Libération mémoire avec coupure Wi-Fi
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

 
