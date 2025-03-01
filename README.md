# ESP32 Battery Tester 🔋  

🚀 **Battery testing system using ESP32, INA226, and Web Server.**  

## 📌 Features  
✅ Measures **battery voltage** and **discharge current** using INA226  
✅ Controls **relay** for load connection  
✅ Logs test results to SPIFFS  
✅ **Web interface** to start/stop tests and configure parameters  

## 🛠️ Hardware  
- **ESP32**  
- **INA226 Voltage & Current Sensor**  
- **Relay module**  
- **Resistor (Load)**  
- **Buzzer**  

## 📷 Screenshot  
![Web Interface](screenshot.png)  

## 🚀 Installation  
1. Clone repo:  
   ```bash
   git clone https://github.com/OleksiiShataliuk/Battery-Capacity-Tester.git
Install Arduino IDE & ESP32 board
Install INA226 library:
Open Library Manager in Arduino IDE
Search Adafruit INA226 and install
Upload code to ESP32
🖥️ Web Interface
Start test: http://192.168.4.1/start
Stop test: http://192.168.4.1/stop
Set Voltage Threshold: http://192.168.4.1/set_threshold
Set Load Resistance: http://192.168.4.1/set_resistance
