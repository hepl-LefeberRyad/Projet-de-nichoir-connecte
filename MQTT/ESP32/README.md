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




