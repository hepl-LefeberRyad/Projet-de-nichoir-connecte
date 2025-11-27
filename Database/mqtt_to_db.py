import mariadb
import paho.mqtt.client as mqtt
import time
import sys

# --- Configuration de la base de données MariaDB ---
DB_HOST = "localhost"       # Adresse du serveur MariaDB
DB_USER = "user"            # Nom d'utilisateur pour la base
DB_PASS = "2001"            # Mot de passe de l'utilisateur
DB_NAME = "IMAGE"           # Nom de la base de données
TABLE_NAME = "messages"     # Nom de la table pour stocker les messages

# --- Configuration MQTT ---
MQTT_BROKER = "192.168.0.194"  # Adresse IP du broker MQTT
MQTT_PORT = 1883                # Port du broker
MQTT_TOPIC = "test/topic"       # Topic auquel on s'abonne

# --- Fonction d'aide : création de la table si elle n'existe pas ---
def create_table():
    try:
        # Connexion à la base de données
        conn = mariadb.connect(
            host=DB_HOST,
            user=DB_USER,
            password=DB_PASS,
            database=DB_NAME
        )
        cur = conn.cursor()

        # Création de la table avec id auto-incrémenté, topic, payload et timestamp
        cur.execute(f"""
            CREATE TABLE IF NOT EXISTS {TABLE_NAME} (
                id INT AUTO_INCREMENT PRIMARY KEY,
                topic VARCHAR(255),
                payload VARCHAR(255),
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        """)

        conn.commit()
        cur.close()
        conn.close()
        print(f"Table `{TABLE_NAME}` prête.")
    except mariadb.Error as e:
        print(f"Erreur lors de la création de la table : {e}")
        sys.exit(1)

# --- Fonction pour insérer un message dans la base ---
def insert_message(topic, payload):
    try:
        # Connexion à la base
        conn = mariadb.connect(
            host=DB_HOST,
            user=DB_USER,
            password=DB_PASS,
            database=DB_NAME
        )
        cur = conn.cursor()

        # Insertion du message dans la table
        cur.execute(
            f"INSERT INTO {TABLE_NAME} (topic, payload) VALUES (?, ?)",
            (topic, payload)
        )

        conn.commit()
        cur.close()
        conn.close()
        print(f"Message enregistré en base : {payload}")
    except mariadb.Error as e:
        print(f"Erreur base de données : {e}")

# --- Callbacks MQTT ---
def on_connect(client, userdata, flags, rc):
    # Appelé lorsque le client se connecte au broker
    if rc == 0:
        print("Connecté au broker MQTT")
        client.subscribe(MQTT_TOPIC)  # Abonnement au topic
    else:
        print(f"Échec de la connexion MQTT, code : {rc}")

def on_message(client, userdata, msg):
    # Appelé à la réception d'un message sur un topic
    payload = msg.payload.decode()
    print(f"Reçu : {payload} sur le topic {msg.topic}")
    insert_message(msg.topic, payload)  # Sauvegarde en base

# --- Programme principal ---
if __name__ == "__main__":
    create_table()  # Création de la table si nécessaire

    # Initialisation du client MQTT
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    # Boucle principale : connexion et écoute des messages
    while True:
        try:
            client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
            client.loop_forever()
        except Exception as e:
            print(f"Connexion MQTT échouée : {e}, nouvelle tentative dans 5 secondes")
            time.sleep(5)
