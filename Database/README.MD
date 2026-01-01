# Database

## Introduction
Ce dossier contient le code et les scripts relatifs à la gestion des données du projet SmartCitiesIoT – Nichoir.  
Il inclut la configuration de la base de données MariaDB, la création de tables pour stocker les messages et les images envoyés par l'ESP32 via MQTT, ainsi que les scripts Python pour insérer et récupérer ces données.

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

