# IoT-Smart-Environment-Monitoring-System

![Status](https://img.shields.io/badge/Status-Completed-success)

## Project Overview
A full-stack IoT system for monitoring environmental conditions (Temperature, Humidity, Light) and automatically controlling cooling fans. The ESP32 ollects sensor data, supports dual networking modes (Station and Access Point) and dual communication protocols (HTTP and MQTT), and sends data to a web backend for storage and machine-learning analytics.

## System Architecture
* **Edge Device:** ESP32 running C++ firmware supporting Station and AP modes.
* **Protocols:**
  * MQTT for real-time telemetry
  * HTTP REST for bulk data upload and synchronization.
* **Backend:** Python MQTT bridge connected to a PHP/MySQL database supporting the web dashboard.
* **Analytics:** Python notebook for correlation analysis and temperature prediction.

## Technology Stack
* **Embedded:** C++, Arduino framework, SPIFFS (Flash Memory), OTA (ArduinoOTA).
* **Networking:** WiFi (Station/AP), MQTT (PubSubClient), HTTP/REST.
* **Backend:** Python (Paho-MQTT), PHP, MySQL.
* **Data Science:** Pandas, Scikit-Learn Seaborn.

## Key Features

* **Over-The-Air (OTA) Updates:** Wireless firmware deployment and debugging.

* **Web Dashboard:** For real-time monitoring and remote control

* **Persistent Data Logging:** Sensor data is cached in Flash memory (SPIFFS) if WiFi is lost and bulk-uploaded upon reconnection.

* **Dual-Protocol Communication:** Switches dynamically between MQTT (for real-time telemetry) and HTTP REST (bulk logging).

* **Dual-Mode Networking:** Auto-switches between station mode and access point Mode.

* **Smart Fan Control:** Fan actuation based on configurable temperature thresholds or a manual web dashboard override.

* **Predictive Analytics:** A linear regression model achieving a 0.75 R² score in predicting temperature from humidity and light levels.
