from flask import Flask, render_template  # Import Flask framework et fonction render_template pour afficher les templates HTML
import mariadb                           # Import pour interagir avec la base de données MariaDB
import base64                            # Import pour encoder les images en Base64
import threading                         # Import pour gérer les threads (MQTT en arrière-plan)
import paho.mqtt.client as mqtt          # Import pour gérer la communication MQTT

app = Flask(__name__)  # Création de l'application Flask

# -------------------- DATABASE --------------------
DB_USER = "user"        # Nom d'utilisateur MariaDB
DB_PASS = "2001"        # Mot de passe MariaDB
DB_HOST = "localhost"   # Hôte de la base de données
DB_NAME = "IMAGE"       # Nom de la base de données
TABLE_NAME = "images"   # Nom de la table contenant les images

# -------------------- MQTT --------------------
MQTT_BROKER = "192.168.0.194"  # Adresse IP du broker MQTT
MQTT_PORT = 1883                # Port MQTT
MQTT_TOPIC_PIC = "pics"        # Topic MQTT pour les images

# -------------------- FUNCTIONS --------------------
def get_images():
    # Connexion à la base de données
    conn = mariadb.connect(user=DB_USER, password=DB_PASS, host=DB_HOST, database=DB_NAME)
    cur = conn.cursor()  # Création d'un curseur pour exécuter les requêtes
    # Récupération des images stockées dans la base, triées par ID décroissant (les plus récentes en premier)
    cur.execute(f"SELECT id, topic, payload, timestamp FROM {TABLE_NAME} ORDER BY id DESC")
    rows = []  # Liste qui contiendra les images formatées
    for id, topic, payload, timestamp in cur.fetchall():  # Parcours de chaque ligne renvoyée par la requête
        img_b64 = base64.b64encode(payload).decode()  # Conversion de l'image (bytes) en chaîne Base64
        rows.append({
            "id": id,               # ID de l'image dans la base
            "topic": topic,         # Topic MQTT de l'image
            "timestamp": timestamp, # Timestamp d'insertion
            "img_b64": img_b64      # Contenu de l'image encodé en Base64
        })
    cur.close()  # Fermeture du curseur
    conn.close() # Fermeture de la connexion à la base
    return rows  # Renvoie la liste des images

# -------------------- MQTT CALLBACKS --------------------
def on_connect(client, userdata, flags, rc):
    # Appelé lorsque le client MQTT se connecte au broker
    if rc == 0:
        print("Connected to MQTT broker")  # Affiche la connexion réussie
        client.subscribe(MQTT_TOPIC_PIC)   # Abonne le client au topic des images
    else:
        print(f"Failed to connect (rc={rc})")  # Affiche un message d'erreur si la connexion échoue

def on_message(client, userdata, msg):
    # Appelé lorsqu'un message MQTT est reçu
    # Pour cette version simplifiée, on ne fait rien avec les messages
    pass

def mqtt_thread():
    # Thread en arrière-plan pour maintenir la connexion MQTT
    client = mqtt.Client()           # Création du client MQTT
    client.on_connect = on_connect   # Définition du callback de connexion
    client.on_message = on_message   # Définition du callback de réception de messages
    while True:                      # Boucle infinie pour garder la connexion
        try:
            client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)  # Connexion au broker MQTT
            client.loop_forever()  # Boucle d'attente et traitement des messages MQTT
        except Exception as e:  # Si erreur de connexion ou autre
            print(f"MQTT error: {e}, reconnecting in 5 seconds...")  # Affiche l'erreur
            import time
            time.sleep(5)  # Attente avant de réessayer la connexion

# -------------------- FLASK ROUTES --------------------
@app.route("/")  # Définition de la route principale de l'application
def index():
    images = get_images()  # Récupère la liste des images depuis la base
    return render_template("index_pics.html", images=images)  # Affiche le template HTML avec les images

# -------------------- MAIN --------------------
if __name__ == "__main__":
    # Démarrage du thread MQTT en arrière-plan
    thread = threading.Thread(target=mqtt_thread, daemon=True)
    thread.start()

    # Démarrage du serveur Flask
    app.run(host="0.0.0.0", port=5001, debug=True)
