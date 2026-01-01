# -*- coding: utf-8 -*-
# Import des modules nécessaires
import os                # Pour la gestion des fichiers et dossiers
import base64            # Pour encoder/décoder les images en base64
import mariadb           # Pour la connexion et manipulation de la base de données MariaDB
import paho.mqtt.client as mqtt  # Pour la communication MQTT
import sys               # Pour quitter le programme en cas d'erreur critique
from datetime import datetime   # Pour obtenir la date et l'heure actuelles
import time              # Pour les temporisations et délais

# -------------------------
# DOSSIERS
# -------------------------
SAVE_FOLDER = "/home/joao1/received_images"  # Dossier où seront sauvegardées les images reçues
os.makedirs(SAVE_FOLDER, exist_ok=True)      # Création du dossier s'il n'existe pas déjà

# -------------------------
# CONFIGURATION MQTT
# -------------------------
MQTT_BROKER = "192.168.178.29"  # Adresse IP du broker MQTT
MQTT_PORT = 1883                 # Port MQTT (par défaut 1883)

MQTT_TOPIC_PIC = "pics"          # Topic MQTT pour les images
MQTT_TOPIC_BATTERY = "battery"   # Topic MQTT pour les informations de batterie

# -------------------------
# CONFIGURATION BASE DE DONNÉES (SEULEMENT IMAGES)
# -------------------------
DB_HOST = "localhost"   # Adresse de la base de données
DB_USER = "user"        # Nom d'utilisateur de la base de données
DB_PASS = "2001"        # Mot de passe de la base de données
DB_NAME = "IMAGE"       # Nom de la base de données
TABLE_NAME = "images"   # Nom de la table où seront stockées les images

# Stockage temporaire des morceaux d'image reçus
image_chunks = {}

# -------------------------
# Création de la table dans la base de données
# -------------------------
def create_table():
    try:
        # Connexion à la base de données
        conn = mariadb.connect(
            host=DB_HOST, user=DB_USER, password=DB_PASS, database=DB_NAME
        )
        cur = conn.cursor()
        # Création de la table si elle n'existe pas déjà
        cur.execute(f"""
            CREATE TABLE IF NOT EXISTS {TABLE_NAME} (
                id INT AUTO_INCREMENT PRIMARY KEY,
                topic VARCHAR(255),
                imageID VARCHAR(255),
                payload LONGBLOB,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        """)
        conn.commit()   # Validation des modifications
        cur.close()     # Fermeture du curseur
        conn.close()    # Fermeture de la connexion
        print("MariaDB: Table prête.")
    except mariadb.Error as e:
        print(f"MariaDB error: {e}")
        sys.exit(1)    # Sortie du programme en cas d'erreur critique

# -------------------------
# Sauvegarde d'une image complète dans la base de données
# -------------------------
def save_to_database(topic, imageID, img_bytes):
    try:
        conn = mariadb.connect(
            host=DB_HOST, user=DB_USER, password=DB_PASS, database=DB_NAME
        )
        cur = conn.cursor()
        # Insertion de l'image dans la table
        cur.execute(
            f"INSERT INTO {TABLE_NAME} (topic, imageID, payload) VALUES (%s, %s, %s)",
            (topic, imageID, img_bytes)
        )
        conn.commit()
        cur.close()
        conn.close()
        print(f"[DB] Image {imageID} sauvegardée ({len(img_bytes)} octets)")
    except mariadb.Error as e:
        print(f"Erreur d'insertion en base: {e}")

# -------------------------
# Sauvegarde de l'image dans le dossier local
# -------------------------
def save_image_file(imageID, img_bytes):
    filename = os.path.join(SAVE_FOLDER, f"{imageID}.jpg")  # Chemin complet du fichier
    with open(filename, "wb") as f:  # Ouverture en mode écriture binaire
        f.write(img_bytes)           # Écriture des octets de l'image
    print(f"[FILE] Image sauvegardée: {filename}")

# -------------------------
# Gestion des messages batterie
# -------------------------
def handle_battery_message(payload):
    """
    Format attendu: "LEVEL,VOLTAGE"
    Exemple: "85,3700"
    """
    try:
        level_str, voltage_str = payload.strip().split(",")  # Séparation niveau/voltage
        battery_level = int(level_str)     # Conversion en entier du niveau
        battery_voltage = int(voltage_str) # Conversion en entier de la tension (mV)
        now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")  # Date et heure actuelle
        print(f"[DEBUG] Données batterie -> Niveau: {battery_level} %, Tension: {battery_voltage} mV à {now}")
    except ValueError:
        print(f"[ERROR] Impossible de parser le message batterie '{payload}'")
    except Exception as e:
        print(f"[ERROR] Erreur inattendue lors du parsing du message batterie '{payload}': {e}")

# -------------------------
# CALLBACKS MQTT
# -------------------------
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connecté au broker MQTT")
        # Souscription aux topics images et batterie
        client.subscribe([
            (MQTT_TOPIC_PIC, 0),
            (MQTT_TOPIC_BATTERY, 0)
        ])
    else:
        print(f"Échec de connexion (rc={rc})")

def on_message(client, userdata, msg):
    topic = msg.topic
    payload = msg.payload.decode(errors="ignore")  # Décodage du payload en chaîne

    # Topic batterie
    if topic == MQTT_TOPIC_BATTERY:
        handle_battery_message(payload)
        return

    # Topic images
    try:
        if payload.count("|") < 3:  # Vérification du format chunké
            print("[INFO] Message MQTT non-image ignoré")
            return

        # Découpage du payload en ID, index, total et chunk
        imageID, index, total, chunk = payload.split("|", 3)
        index = int(index)
        total = int(total)

        # Initialisation du dictionnaire pour cette image
        if imageID not in image_chunks:
            image_chunks[imageID] = {}

        # Stockage du chunk
        image_chunks[imageID][index] = chunk

        # Vérification si tous les chunks ont été reçus
        if len(image_chunks[imageID]) == total:
            print(f"[INFO] Image complète: {imageID}")

            # Reconstruction de l'image complète et décodage base64
            full_b64 = "".join(image_chunks[imageID][i] for i in range(total))
            img_bytes = base64.b64decode(full_b64)

            # Sauvegarde fichier local et base de données
            save_image_file(imageID, img_bytes)
            save_to_database(topic, imageID, img_bytes)

            # Suppression de l'image du stockage temporaire
            del image_chunks[imageID]

    except Exception as e:
        print(f"Erreur lors du traitement de l'image: {e}")

# -------------------------
# BOUCLE PRINCIPALE
# -------------------------
if __name__ == "__main__":
    create_table()  # Création de la table si elle n'existe pas

    client = mqtt.Client()         # Création du client MQTT
    client.on_connect = on_connect # Assignation du callback de connexion
    client.on_message = on_message # Assignation du callback de réception de messages

    while True:
        try:
            client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)  # Connexion au broker
            client.loop_forever()  # Boucle infinie pour écouter les messages
        except Exception as e:
            print(f"Erreur MQTT: {e}, reconnexion dans 5 secondes...")
            time.sleep(5)  # Attente avant nouvelle tentative de connexion
