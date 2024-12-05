# ESP32 IoT Application with AWS Integration

This repository contains the firmware for an ESP32 microcontroller that collects temperature and humidity data using a **DHT11 sensor** and securely sends the data to **AWS IoT Core** via **MQTT**. The data is processed by an **AWS Lambda function** and stored in **DynamoDB**, enabling real-time monitoring and analytics.

## 📖 Overview

The ESP32 collects temperature and humidity data from the DHT11 sensor at regular intervals. This data is sent to AWS IoT Core using the MQTT protocol. A message rule in IoT Core forwards the data to a Lambda function, which processes and associates the data with an owner before storing it in a DynamoDB table.

## 🛠️ Features

- **Sensor Integration**: Reads temperature and humidity from a DHT11 sensor.
- **AWS IoT Core**: Securely connects and sends data using MQTT.
- **AWS Lambda**: Processes incoming data and applies transformations.
- **AWS DynamoDB**: Stores processed data for real-time and historical analysis.
- **Device Shadow**: Allows remote configuration of the ESP32, such as changing the data transmission interval.

## 📋 Requirements

- **Hardware**:
  - ESP32 microcontroller
  - DHT11 sensor
  - Internet connection
- **Software**:
  - Arduino IDE or PlatformIO
  - Required libraries:
    - `WiFiClientSecure`
    - `MQTTClient`
    - `ArduinoJson`
    - `DHT`

## 🔧 Setup Instructions

1. **Configure AWS IoT Core**:
   - Create an IoT Thing and download its certificates.
   - Set up a message rule to forward data to a Lambda function.

2. **Set Up the ESP32**:
   - Connect the DHT11 sensor to the ESP32 (default pin: `D4`).
   - Clone this repository and open the code in your preferred IDE.
   - Replace placeholders in the `secrets.h` file with your Wi-Fi credentials, AWS IoT endpoint, and certificate paths.

3. **Upload the Firmware**:
   - Install the required libraries in your IDE.
   - Compile and upload the code to your ESP32.

4. **Run and Monitor**:
   - Power on the ESP32 and monitor the serial console for logs.
   - Verify data flow to AWS IoT Core and Lambda.

## 📂 File Structure

- `main.ino`: Main application logic for the ESP32.
- `secrets.h`: Configuration file for Wi-Fi credentials and AWS certificates.
- `handler.ts` (Lambda function): Processes IoT data and stores it in DynamoDB.

## 📜 License

This project is licensed under the MIT License. See the `LICENSE` file for details.

## 📧 Contact

For support or questions, feel free to reach out at [Your Contact Info].
