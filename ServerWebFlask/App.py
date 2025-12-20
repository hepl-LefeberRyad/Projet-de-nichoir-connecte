from flask import Flask, render_template
import mariadb
import base64
import threading
import paho.mqtt.client as mqtt

app = Flask(__name__)

# -------------------- DATABASE --------------------
DB_USER = "user"
DB_PASS = "2001"
DB_HOST = "localhost"
DB_NAME = "IMAGE"
TABLE_NAME = "images"

# -------------------- MQTT --------------------
MQTT_BROKER = "192.168.0.194"
MQTT_PORT = 1883
MQTT_TOPIC_PIC = "pics"

# -------------------- FUNCTIONS --------------------
def get_images():
    conn = mariadb.connect(user=DB_USER, password=DB_PASS, host=DB_HOST, database=DB_NAME)
    cur = conn.cursor()
    cur.execute(f"SELECT id, topic, payload, timestamp FROM {TABLE_NAME} ORDER BY id DESC")
    rows = []
    for id, topic, payload, timestamp in cur.fetchall():
        img_b64 = base64.b64encode(payload).decode()  # Convert bytes -> Base64 string
        rows.append({
            "id": id,
            "topic": topic,
            "timestamp": timestamp,
            "img_b64": img_b64
        })
    cur.close()
    conn.close()
    return rows

# -------------------- MQTT CALLBACKS --------------------
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker")
        client.subscribe(MQTT_TOPIC_PIC)
    else:
        print(f"Failed to connect (rc={rc})")

def on_message(client, userdata, msg):
    # For this simplified version, we don't do anything with MQTT messages
    pass

def mqtt_thread():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    while True:
        try:
            client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
            client.loop_forever()
        except Exception as e:
            print(f"MQTT error: {e}, reconnecting in 5 seconds...")
            import time
            time.sleep(5)

# -------------------- FLASK ROUTES --------------------
@app.route("/")
def index():
    images = get_images()
    return render_template("index_pics.html", images=images)

# -------------------- MAIN --------------------
if __name__ == "__main__":
    # Start MQTT listener in background thread
    thread = threading.Thread(target=mqtt_thread, daemon=True)
    thread.start()

    app.run(host="0.0.0.0", port=5001, debug=True)
