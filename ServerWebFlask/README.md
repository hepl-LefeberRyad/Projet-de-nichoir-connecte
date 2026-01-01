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
