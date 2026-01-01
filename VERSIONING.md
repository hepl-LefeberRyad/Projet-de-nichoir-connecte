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
