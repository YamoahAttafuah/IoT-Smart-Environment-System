# IoT-Smart-Environment-Monitoring-System

![Status](https://img.shields.io/badge/Status-Completed-success)
![Tech](https://img.shields.io/badge/Stack-ESP32%20|%20MQTT%20|%20Python%20|%20ML-blue)

## Project Overview
This project is a full-stack IoT system designed to monitor environmental conditions (Temperature, Humidity, Light) and control fans using the ESP32 platform. It features a dual-protocol architecture (switching between HTTP and MQTT), a custom backend, and a Linear Regression model to predict environmental trends.

## System Architecture
* **Edge Device:** ESP32 running C++ firmware (with station and AP modes).
* **Protocols:** MQTT (PubSubClient) for real-time telemetry, HTTP for bulk data logging.
* **Backend:** Python script connecting MQTT broker to a PHP/MySQL database.
* **Analytics:** Python notebook to analyze sensor correlations and predict temperature spikes.

## Components
* **Embedded:** C++, Arduino framework, SPIFFS (Flash Memory), OTA Updates.
* **Connectivity:** WiFi (Station/AP), MQTT, HTTP/REST.
* **Backend:** Python (Paho-MQTT), PHP, MySQL.
* **Data Science:** Pandas, Seaborn, Scikit-Learn (for the Linear Regression model).

## Key Features

* **Over-The-Air (OTA) Updates:** Wireless firmware flashing and debugging using ArduinoOTA.

* **Persistent Data Logging:** Sensor data is cached in Flash memory (SPIFFS) if WiFi is lost and bulk-uploaded upon reconnection.

* **Dual-Protocol Communication:** Toggles between MQTT (for real-time telemetry) and HTTP REST (bulk logging) via remote commands.

* **Dual-Mode Networking:** Auto-switches between station mode (connecting to router) and access point Mode (acting as a router).

* **Fan Actuation:** Fan actuation based on configurable temperature thresholds or a manual web-interface override.

* **Predictive Analytics:** A Linear Regression model with a 0.75 R² score in predicting temperature based on humidity and light levels.
