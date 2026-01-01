# ServerWebFlask

## Introduction
Dans cette partie du répertoire, nous décrivons notre progression avec l'application d'un serveur web pour notre projet. Chaque patch correspond à une évolution du code pour intégrer progressivement les fonctionnalités.

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
