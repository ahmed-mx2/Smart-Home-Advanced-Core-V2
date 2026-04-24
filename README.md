# 🏠 Advanced IoT Smart Home Core V2.0
**An Integrated System for Automated Home Control & Real-time Telemetry**

---

## 👨‍💻 Developed By
**Engineer: Ahmed Mohamed Mohamed Ahmed** *Telecommunications & Electronics Engineering*

---

## 🚀 Project Overview
This project is an advanced IoT-based Smart Home solution built on the **ESP32** architecture. It features an asynchronous web server, real-time sensor integration (PIR & LDR), and a dynamic control dashboard using **SSE (Server-Sent Events)** for live data streaming.

---

## 🎓 Academic Integration (Theory to Practice)
This project serves as a practical implementation for three core engineering disciplines:

### 1️⃣ Information & Coding Theory
* **Source Coding & Entropy:** The system treats sensor inputs (PIR/LDR) as **Discrete Information Sources**. Using **JSON Payload Formatting**, we implement source coding to minimize redundancy and represent device states with the least number of bits, optimizing bandwidth.
* **Channel Coding & Reliability:** Data transmission over the 2.4GHz Wi-Fi band utilizes **Forward Error Correction (FEC)** and **CRC-32 (Cyclic Redundancy Check)** at the MAC layer. This ensures the integrity of control commands (High Reliability) against channel noise.

### 2️⃣ Digital Communication
* **Adaptive Modulation Schemes:** The system relies on the 802.11n standard, which employs **OFDM (Orthogonal Frequency Division Multiplexing)**. The ESP32 dynamically switches between **BPSK, QPSK, and QAM (16/64)** based on the **SNR (Signal-to-Noise Ratio)** to maintain the **Shannon Channel Capacity**.
* **RSSI Telemetry:** Integrated real-time monitoring of **RSSI (Received Signal Strength Indicator)** in dBm, allowing for analysis of signal fading and path loss in indoor environments.

### 3️⃣ Microprocessors Interfacing
* **Hardware Interfacing:** Direct interfacing with Digital sensors (HC-SR501 PIR) and Analog/Digital modules (LDR) via **GPIO Mapping**.
* **Asynchronous Processing:** Utilizing an **Asynchronous Web Server (AsyncTCP)** to handle multiple client requests without blocking the CPU's main loop (Non-blocking I/O).
* **Actuator Control:** Implementing GPIO-based switching for a 4-channel relay module, utilizing internal **Pull-up/Pull-down** resistors for signal stability.

---

## 🛠️ Tech Stack
* **Microcontroller:** ESP32 (Dual Core).
* **Communication Protocols:** HTTP, SSE, mDNS, WebSockets.
* **Frontend:** HTML5, CSS3 (Custom Dark Theme), JavaScript (ES6).
* **Libraries:** `ESPAsyncWebServer`, `AsyncTCP`, `WiFi.h`.

---

## 📦 System Architecture
The system is built on a **Non-blocking Event-Driven Architecture**. Unlike traditional polling methods, this core uses **Server-Sent Events (SSE)** to push updates to the client only when a state change occurs, significantly reducing overhead.

---

## ⚖️ License
**Authorized Source Code - Proprietary Access.** Copyright © 2026 **Ahmed Mohamed Mohamed Ahmed**. All rights reserved.
