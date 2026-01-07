import time
import requests
import paho.mqtt.client as mqtt

MQTT_SERVER = "mqtt-dashboard.com"
MQTT_TOPICS = ["fp/temperature", "fp/humidity", "fp/ldr"]
url = 'http://localhost/iot/final_project/insert.php'

# The callback when connected.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    for topic in MQTT_TOPICS:
        client.subscribe(topic)

# Callback when message received
def on_message(client, userdata, msg):
    msg.payload = msg.payload.decode('utf-8')
    print("Message: " + msg.payload)
    print ("Message received from " + str(msg.topic))
    post(msg.payload)

def post(message):
    data = dict(subString.split(":") for subString in str(message).split(","))
    response = requests.post(url, json=data)
    print("Status code: ", response.status_code)
    print("Response: " + str(response.content) + "\n")

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
client.on_connect = on_connect
client.on_message = on_message
client.connect(MQTT_SERVER, 1883, 60)

print("waiting for messages")
client.loop_forever()