import mariadb
import paho.mqtt.client as mqtt
import base64
import sys
import os
from datetime import datetime

# --- Configuration de la base de données MariaDB ---
DB_HOST = "localhost"       # Adresse du serveur MariaDB
DB_USER = "user"            # Nom d'utilisateur pour la base
DB_PASS = "2001"            # Mot de passe
DB_NAME = "IMAGE"           # Nom de la base de données
TABLE_NAME = "images"       # Nom de la table pour stocker les images

# --- Configuration MQTT ---
MQTT_BROKER = "192.168.0.194"  # Adresse IP du broker MQTT
MQTT_PORT = 1883                # Port du broker
MQTT_TOPIC = "pics"             # Topic auquel le script s'abonne

# --- Dossier pour sauvegarder les images reçues ---
SAVE_FOLDER = "/home/joao1/received_images"
os.makedirs(SAVE_FOLDER, exist_ok=True)  # Crée le dossier s'il n'existe pas

# --- Fonction utilitaire : création de la table si elle n'existe pas ---
def create_table():
    try:
        # Connexion à la base de données
        conn = mariadb.connect(host=DB_HOST, user=DB_USER, password=DB_PASS, database=DB_NAME)
        cur = conn.cursor()

        # Création de la table avec id auto-incrémenté, topic, payload et timestamp
        cur.execute(f"""
            CREATE TABLE IF NOT EXISTS {TABLE_NAME} (
                id INT AUTO_INCREMENT PRIMARY KEY,
                topic VARCHAR(255),
                payload LONGBLOB,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        """)
        conn.commit()
        cur.close()
        conn.close()
        print(f"Table `{TABLE_NAME}` prête.")
    except mariadb.Error as e:
        print(f"Erreur base de données : {e}")
        sys.exit(1)

# --- Fonction pour insérer une image dans la base de données ---
def insert_image(topic, img_bytes):
    try:
        # Connexion à la base
        conn = mariadb.connect(host=DB_HOST, user=DB_USER, password=DB_PASS, database=DB_NAME)
        cur = conn.cursor()

        # Insertion du payload sous forme de BLOB
        cur.execute(
            f"INSERT INTO {TABLE_NAME} (topic, payload) VALUES (%s, %s)",
            (topic, img_bytes)
        )

        conn.commit()
        cur.close()
        conn.close()
        print(f"Image enregistrée en base ({len(img_bytes)} bytes)")

    except mariadb.Error as e:
        print(f"Erreur base de données : {e}")

# --- Fonction pour sauvegarder l'image localement ---
def save_image_file(img_bytes):
    try:
        # Nom de fichier basé sur la date et l'heure
        filename = datetime.now().strftime("%Y%m%d_%H%M%S") + ".jpg"
        filepath = os.path.join(SAVE_FOLDER, filename)

        # Écriture du fichier
        with open(filepath, "wb") as f:
            f.write(img_bytes)

        print(f"Image sauvegardée dans le dossier : {filepath}")

    except Exception as e:
        print(f"Erreur lors de la sauvegarde du fichier : {e}")

# --- Callbacks MQTT ---
def on_connect(client, userdata, flags, rc):
    # Appelé lorsque le client se connecte au broker
    if rc == 0:
        print("Connecté au broker MQTT")
        client.subscribe(MQTT_TOPIC)  # Abonnement au topic
    else:
        print(f"Échec de la connexion MQTT, code {rc}")

def on_message(client, userdata, msg):
    # Appelé lorsqu'un message est reçu sur le topic abonné
    try:
        # Décodage du payload Base64 avec correction du padding
        img_b64 = msg.payload.decode()  # Conversion bytes -> string
        img_b64 += "=" * (-len(img_b64) % 4)  # Correction du padding
        img_bytes = base64.b64decode(img_b64)  # Décodage Base64 en bytes

        # Sauvegarde dans la base de données
        insert_image(msg.topic, img_bytes)

        # Sauvegarde dans le dossier local
        save_image_file(img_bytes)

    except Exception as e:
        print(f"Erreur décodage/sauvegarde image : {type(e).__name__}: {e}")

# --- Programme principal ---
if __name__ == "__main__":
    create_table()  # Création de la table si nécessaire

    # Initialisation du client MQTT
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    # Boucle principale : connexion au broker et réception des messages
    while True:
        try:
            client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
            client.loop_forever()
        except Exception as e:
            print(f"Connexion MQTT échouée : {type(e).__name__}: {e}, nouvelle tentative dans 5s")
            time.sleep(5)
