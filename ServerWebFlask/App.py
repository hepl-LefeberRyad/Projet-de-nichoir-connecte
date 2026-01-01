# -*- coding: utf-8 -*-
from flask import Flask, render_template  # Import de Flask pour le serveur web et le rendu de templates HTML
import mariadb                             # Import de MariaDB pour se connecter à la base de données
import base64                              # Import pour encoder/décoder les images en base64
import threading                            # Import pour exécuter le client MQTT dans un thread séparé
import paho.mqtt.client as mqtt             # Import du client MQTT
from datetime import datetime               # Import pour manipuler les dates et heures
import time                                 # Import pour utiliser sleep

# Initialisation de l'application Flask
app = Flask(__name__)

# -------------------- DATABASE --------------------
# Paramètres de connexion à la base de données MariaDB
DB_USER = "user"
DB_PASS = "2001"
DB_HOST = "localhost"
DB_NAME = "IMAGE"
TABLE_NAME = "images"

# -------------------- MQTT --------------------
# Paramètres de connexion au broker MQTT
MQTT_BROKER = "192.168.0.194"
MQTT_PORT = 1883
MQTT_TOPIC_BATTERY = "battery"  # Topic pour les données batterie

# Stockage des dernières informations de batterie reçues
battery_info = {
    "voltage": None,    # Tension en volts
    "capacity": None,   # Pourcentage de batterie
    "timestamp": None   # Date et heure de la dernière mise à jour
}

# -------------------- FUNCTIONS --------------------
def get_images():
    """Récupère toutes les images depuis MariaDB et les convertit en base64 pour l'affichage dans HTML"""
    conn = mariadb.connect(user=DB_USER, password=DB_PASS, host=DB_HOST, database=DB_NAME)  # Connexion DB
    cur = conn.cursor()  # Création d'un curseur
    cur.execute(f"SELECT id, topic, payload, timestamp FROM {TABLE_NAME} ORDER BY id DESC")  # Requête pour récupérer les images
    rows = []
    for id, topic, payload, timestamp in cur.fetchall():  # Parcours des résultats
        img_b64 = base64.b64encode(payload).decode()      # Convertit les bytes de l'image en string base64
        rows.append({
            "id": id,
            "topic": topic,
            "timestamp": timestamp,
            "img_b64": img_b64
        })
    cur.close()   # Fermeture du curseur
    conn.close()  # Fermeture de la connexion
    return rows   # Retourne la liste des images avec infos

# -------------------- MQTT CALLBACKS --------------------
def on_connect(client, userdata, flags, rc):
    """Callback exécuté lors de la connexion au broker MQTT"""
    if rc == 0:
        print("Connected to MQTT broker")
        client.subscribe(MQTT_TOPIC_BATTERY)  # S'abonne au topic batterie
    else:
        print(f"Failed to connect (rc={rc})")  # Affiche un message en cas d'erreur

def on_message(client, userdata, msg):
    """Callback exécuté à la réception d'un message MQTT"""
    global battery_info
    try:
        payload = msg.payload.decode().strip()  # Decode le message reçu
        # Attendu: format "niveau,voltage" par ex. "85,3700"
        if "," in payload:
            level_str, voltage_str = payload.split(",")      # Sépare niveau et voltage
            battery_info["capacity"] = int(level_str)       # Stocke le niveau de batterie
            battery_info["voltage"] = int(voltage_str) / 1000.0  # Convertit mV en V
            battery_info["timestamp"] = datetime.now().strftime("%Y-%m-%d %H:%M:%S")  # Stocke la date de réception
            print(f"[DEBUG] Updated battery info: {battery_info}")  # Debug
    except Exception as e:
        print(f"[ERROR] Parsing battery message '{payload}': {e}")  # Affiche les erreurs

def mqtt_thread():
    """Exécute le client MQTT dans un thread séparé pour ne pas bloquer Flask"""
    client = mqtt.Client()           # Création d'un client MQTT
    client.on_connect = on_connect   # Définit la fonction de callback connect
    client.on_message = on_message   # Définit la fonction de callback message
    while True:
        try:
            client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)  # Connexion au broker
            client.loop_forever()  # Boucle infinie pour recevoir les messages
        except Exception as e:
            print(f"MQTT error: {e}, reconnecting in 5 seconds...")  # Affiche les erreurs
            time.sleep(5)  # Attend 5 secondes avant de retenter

# -------------------- FLASK ROUTES --------------------
@app.route("/")
def index():
    """Route principale, affiche la page HTML avec images et batterie"""
    images = get_images()  # Récupère les images depuis la DB
    return render_template("index_pics.html", images=images, battery=battery_info)  # Passe les données au template

# -------------------- MAIN --------------------
if __name__ == "__main__":
    # Démarre le listener MQTT dans un thread en arrière-plan
    thread = threading.Thread(target=mqtt_thread, daemon=True)
    thread.start()

    # Démarre le serveur Flask
    app.run(host="0.0.0.0", port=5001, debug=True)
