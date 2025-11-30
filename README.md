# WiFi DC Motor Controller (Arduino + ESP8266 + Python GUI + Android App)
**Note:** Python must be installed on your laptop.
## 📥 Download APK
[➡️ **Download Latest Android APK**](https://github.com/jainishpatel2106/MotorController-IoT/releases/latest)

[![Download APK](https://img.shields.io/badge/Download-APK-blue?style=for-the-badge&logo=android)](https://github.com/jainishpatel2106/MotorController-IoT/releases/latest)

Control a 12V DC motor wirelessly using Arduino Mega/Uno, ESP8266 module, L293D motor driver, Python GUI, and Android APK.

This project works entirely on a local Wi-Fi LAN (no internet required).

---

## Features
- Control motor speed (0–100%)
- Change direction (Forward / Reverse)
- Safe direction switching (automatic motor stop before reversing)
- Control via:
  - Python GUI (Laptop)
  - Android App (APK included)
- ESP8266 acts as a TCP server
- Real-time response
- Open-source project

---

## Hardware Required  
*(Sample images in /Images/ folder)*

| Component                  | Quantity | Notes                                      |
|----------------------------|----------|--------------------------------------------|
| Arduino Mega    or         |    1     | Serial1 used for ESP communication         |
| Arduino Uno                |    1     | SoftwareSerial required                    |
| ESP8266 (ESP-01 or ESP-12) |    1     | Must have AT firmware                      |
| L293D Motor Driver Module  |    1     | For 12V DC motor                           |
| 12V DC Motor               |    1     | Brushed motor                              |
| 12V Power Supply           |    1     | For driver + motor                         |
| Jumper wires               |    –     | Standard connections                       |

---

## Wiring Diagram

### ESP8266 → Arduino Mega
- ESP8266 VCC → 3.3V  
- ESP8266 CH_PD (EN) → 3.3V  
- ESP8266 RX → TX1 (Pin 18) *(use 1k series resistor recommended)*  
- ESP8266 TX → RX1 (Pin 19)  
- ESP8266 GND → GND  

### ESP8266 → Arduino Uno (SoftwareSerial)
- ESP8266 TX → Arduino Pin **2** (SoftwareSerial RX)  
- ESP8266 RX → Arduino Pin **3** (SoftwareSerial TX, via 1k resistor)  
- ESP826826 VCC → 3.3V  
- ESP8266 CH_PD → 3.3V  
- GND must be common with Arduino & driver  

### L293D Motor Driver
- Motor Driver GND → Arduino GND  
- Motor Driver 12V → Same 12V adaptor powering Arduino  
- Motor output → L293D OUT1 / OUT2

(Refer to wiring.png in the /Images folder)

---

## Initial Step – Upload ESP_setup.ino  
**Location:** `/Arduino/ESP_setup.ino`

This must be uploaded BEFORE uploading the main motor controller code.

### For Arduino Uno:
Add before setup():

#include <SoftwareSerial.h>
SoftwareSerial esp(2, 3);  // RX=2, TX=3

In setup():

Serial.begin(115200);
esp.begin(9600);

Replace all Serial1 with esp.
Do the same thing mentioned above also in the main code.

### For Arduino Mega:
Use code exactly as provided.

### After uploading, open Serial Monitor:

AT      -    to check the comm between Arduino and ESP        
AT+RST  -    To reset the ESP
AT+CWMODE=1   -  To set ESP in station mode
AT+CWJAP="YourWiFiName","YourPassword"  -  To connect to your router
AT+CIFSR  ← Note the IP address  

Use this IP in Python GUI & Android App.

---

## Arduino Motor Controller to be uploaded to Arduino.  
**Location:** `/Arduino/motor_controller.ino`

This file:
- Receives TCP data from ESP8266  
- Splits speed,direction  
- Converts speed → PWM  
- Controls L293D IN1/IN2  
- Enforces 1-second safety delay before reversing  
- Fully commented for beginners

---

## ESP8266 Auto Configuration (sent by Arduino automatically)

AT+CIPMUX=1  
AT+CIPSERVER=1,5050  
AT+CIPSTO=0  

This makes ESP8266 a:
- TCP Server  
- Port 5050  
- No timeout  
- Multi-client enabled  

---

## Python GUI Controller

**Location:** `/Python/motor_controller_GUI.py`  
Run:

python motor_controller_GUI.py

If manually creating file:  
Paste code → save → rename from .txt to .py

### Features:
- Slider  
- + / – buttons  
- Start/Stop  
- Forward/Reverse  
- 1-second debounce  
- Stable TCP communication  

---

## Android App

Source Code:  
`/Android/MotorController/`

APK File:  
`/APK/MotorController.apk`

### App Features:
- Connect using ESP IP & Port  
- Slider control  
- Direction control  
- Start/Stop  
- Same behavior as Python GUI  
- Safe direction switching  

---

## Project Folder Structure

MotorController-IoT/  
│  
├── Arduino/  
│   ├── motor_controller.ino  
│   └── ESP_setup.ino  
│  
├── Python/  
│   └── motor_controller_GUI.py  
│  
├── Android/  
│   └── MotorController/  
│       ├── app/  
│       ├── gradle/  
│       ├── build.gradle.kts  
│       ├── settings.gradle.kts  
│       ├── gradlew  
│       └── gradlew.bat  
│  
├── APK/  
│   └── MotorController.apk  
│  
├── Images/  
│   ├── wiring.png  
│   ├── screenshot.png  
│   └── sample.png  
│  
└── README.md  

---

## How Everything Works

1. Python/Android sends TCP packet: `speed,direction`  
   Example:  
   40,fwd  
   0,fwd  
   75,rev  

2. ESP8266 forwards packet to Arduino  
3. Arduino parses & controls motor  
4. L293D drives the motor  

---

## Safety Feature: Direction Reversal Protection
- PWM set to 0  
- Waits 1 second  
- Switches direction  

Prevents:
- Back EMF  
- Shoot-through  
- Current surge  
- Mechanical shock  

---

## Contributing  
Pull requests welcome.

---

## Credits  
Created by Jainish Patel  

## Contact
For questions or feedback, open an issue on GitHub:
https://github.com/jainishpatel2106/MotorController-IoT/issues


<p align="left">
  <a href="./LICENSE">
    <img src="https://img.shields.io/badge/License-MIT-yellow.svg" />
  </a>
  <a href="https://github.com/jainishpatel2106/MotorController-IoT/releases/latest">
    <img src="https://img.shields.io/github/v/release/jainishpatel2106/MotorController-IoT?color=blue&label=Latest%20Release" />
  </a>
  <a href="https://github.com/jainishpatel2106/MotorController-IoT">
    <img src="https://img.shields.io/github/stars/jainishpatel2106/MotorController-IoT?style=social" />
  </a>
</p>

------------------------------------------------------------

⚠️ Hardware Wiring Disclaimer

The hardware wiring diagrams, motor driver instructions, electrical
connections, ESP8266 setup details, and power distribution guidance provided
in this project are intended for educational and experimental use only.
Incorrect wiring, improper power supply handling, or misuse of motor drivers
and ESP modules may result in component damage, electrical hazards, fire
risks, or personal injury.

By using this project, you agree that you are solely responsible for verifying
all electrical connections and ensuring safe electrical practices. You assume
all risks associated with assembling, wiring, and powering the hardware
described in this repository.

The author assumes no responsibility or liability for any damage, loss, or
injury resulting from the use of the hardware information or instructions
provided in this project.
