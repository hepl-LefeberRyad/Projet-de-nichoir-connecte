import os                     # Pour gérer les dossiers et fichiers
import base64                 # Pour encoder/décoder les images en base64
import mariadb                # Pour interagir avec MariaDB
import paho.mqtt.client as mqtt  # Pour communiquer via MQTT
import sys                    # Pour gérer les sorties/sorties en cas d'erreur
from datetime import datetime  # Pour générer des noms de fichiers avec timestamp

# -------------------------
# DOSSIERS
# -------------------------
SAVE_FOLDER = "/home/joao1/received_images"  # Chemin du dossier où on sauvegarde les images
os.makedirs(SAVE_FOLDER, exist_ok=True)      # Crée le dossier si il n'existe pas

# -------------------------
# PARAMÈTRES MQTT
# -------------------------
MQTT_BROKER = "192.168.0.194"  # Adresse IP du broker MQTT
MQTT_PORT = 1883               # Port MQTT par défaut
MQTT_TOPIC = "pics"            # Topic sur lequel on reçoit les images

# -------------------------
# PARAMÈTRES BASE DE DONNÉES
# -------------------------
DB_HOST = "localhost"   # Adresse de la base de données
DB_USER = "user"        # Nom d'utilisateur
DB_PASS = "2001"        # Mot de passe
DB_NAME = "IMAGE"       # Nom de la base de données
TABLE_NAME = "images"   # Nom de la table pour stocker les images

# Stockage temporaire des morceaux d'images
# On utilise un dictionnaire pour chaque imageID
# Structure : {imageID: {index_chunk: chunk}}
image_chunks = {}

# -------------------------
# CRÉATION DE LA TABLE DANS LA BASE DE DONNÉES
# -------------------------
def create_table():
    try:
        # Connexion à la base de données
        conn = mariadb.connect(
            host=DB_HOST, user=DB_USER, password=DB_PASS, database=DB_NAME
        )
        cur = conn.cursor()  # Création du curseur pour exécuter les commandes SQL

        # Requête SQL : crée la table si elle n'existe pas
        cur.execute(f"""
            CREATE TABLE IF NOT EXISTS {TABLE_NAME} (
                id INT AUTO_INCREMENT PRIMARY KEY,   # ID auto-incrémenté
                topic VARCHAR(255),                  # Topic MQTT
                imageID VARCHAR(255),                # ID unique de l'image
                payload LONGBLOB,                    # Contenu de l'image en BLOB
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP  # Date/heure d'enregistrement
            )
        """)
        conn.commit()   # Valide la création de la table
        cur.close()     # Ferme le curseur
        conn.close()    # Ferme la connexion
        print("MariaDB: Table prête.")  # Confirmation console

    except mariadb.Error as e:
        print(f"Erreur MariaDB: {e}")  # Affiche l'erreur
        sys.exit(1)                     # Quitte le programme en cas d'erreur

# -------------------------
# ENREGISTRER UNE IMAGE DANS LA BASE DE DONNÉES
# -------------------------
def save_to_database(topic, imageID, img_bytes):
    try:
        # Connexion à la base de données
        conn = mariadb.connect(
            host=DB_HOST, user=DB_USER, password=DB_PASS, database=DB_NAME
        )
        cur = conn.cursor()  # Curseur pour exécuter les requêtes

        # Requête SQL pour insérer l'image
        cur.execute(
            f"INSERT INTO {TABLE_NAME} (topic, imageID, payload) VALUES (%s, %s, %s)",
            (topic, imageID, img_bytes)  # Données à insérer
        )

        conn.commit()   # Valide l'insertion
        cur.close()     # Ferme le curseur
        conn.close()    # Ferme la connexion
        print(f"[DB] Image {imageID} enregistrée dans MariaDB ({len(img_bytes)} bytes)")  # Confirmation

    except mariadb.Error as e:
        print(f"Erreur insertion DB: {e}")  # Affiche l'erreur si insertion échoue

# -------------------------
# ENREGISTRER UNE IMAGE DANS LE DOSSIER LOCAL
# -------------------------
def save_image_file(imageID, img_bytes):
    filename = os.path.join(SAVE_FOLDER, f"{imageID}.jpg")  # Chemin complet du fichier
    try:
        with open(filename, "wb") as f:  # Ouvre le fichier en mode écriture binaire
            f.write(img_bytes)            # Écrit les bytes de l'image
        print(f"[FILE] Image sauvegardée dans {filename}")  # Confirmation console
    except Exception as e:
        print(f"Erreur sauvegarde fichier: {e}")  # Affiche l'erreur si écriture échoue

# -------------------------
# GESTIONNAIRES MQTT
# -------------------------
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connecté au broker MQTT")  # Connexion réussie
        client.subscribe(MQTT_TOPIC)      # S'abonne au topic
    else:
        print(f"Échec de connexion (rc={rc})")  # Affiche code retour si échec

def on_message(client, userdata, msg):
    global image_chunks  # On modifie la variable globale

    try:
        payload = msg.payload.decode()  # Convertit le message en chaîne de caractères

        # Format attendu : imageID|index|total|base64chunk
        if payload.count("|") < 3:  # Si moins de 3 séparateurs, le message est ignoré
            print("[WARN] Message MQTT non conforme ignoré")
            return

        # Séparer le message en ses parties
        imageID, index, total, chunk = payload.split("|", 3)
        index = int(index)  # Convertit l'index en entier
        total = int(total)  # Convertit le total en entier

        # Préparer le dictionnaire pour cette image si il n'existe pas
        if imageID not in image_chunks:
            image_chunks[imageID] = {}

        # Stocker le chunk
        image_chunks[imageID][index] = chunk

        # Vérifie si tous les chunks sont reçus
        if len(image_chunks[imageID]) == total:
            print(f"[INFO] Tous les chunks reçus pour l'image {imageID}")

            # Réassembler les chunks dans le bon ordre
            full_b64 = "".join(image_chunks[imageID][i] for i in range(total))

            # Décoder le Base64 pour obtenir les bytes de l'image
            img_bytes = base64.b64decode(full_b64)

            # Sauvegarder l'image dans le dossier local
            save_image_file(imageID, img_bytes)

            # Sauvegarder l'image dans la base de données
            save_to_database(msg.topic, imageID, img_bytes)

            # Libérer la mémoire pour cette image
            del image_chunks[imageID]

    except Exception as e:
        print(f"Erreur traitement chunk: {e}")  # Affiche les erreurs éventuelles

# -------------------------
# BOUCLE PRINCIPALE
# -------------------------
if __name__ == "__main__":
    create_table()  # Crée la table si elle n'existe pas

    client = mqtt.Client()          # Crée un client MQTT
    client.on_connect = on_connect  # Définit le callback de connexion
    client.on_message = on_message  # Définit le callback de réception de message

    while True:  # Boucle infinie
        try:
            client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)  # Connexion au broker
            client.loop_forever()  # Boucle MQTT : écoute et appelle les callbacks
        except Exception as e:
            print(f"Erreur MQTT: {e}, reconnexion dans 5 secondes...")  # En cas d'erreur
