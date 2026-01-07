import time
import paho.mqtt.publish as publish

MQTT_SERVER = "mqtt-dashboard.com"
control_topic = "fp/control"
protocol_topic = "fp/protocol"

control = "0" # 0 for HTTP, 1 for MQTT
protocol = "0" # 0 for Station, 1 for AP
 
while True: 
	publish.single(control_topic, control, hostname=MQTT_SERVER)
	publish.single(protocol_topic, protocol, hostname=MQTT_SERVER)
	time.sleep(2)