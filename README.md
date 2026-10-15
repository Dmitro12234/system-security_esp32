**#🏠  system-security**

**YouTube:** https://youtu.be/FZ1HZD5ZdE0

A smart home security system powered by an ESP32 DevKit V1 microcontroller, a PIR motion sensor, an LED, and a buzzer. The project detects motion, triggers an audible and visual alarm, and provides a web interface for remotely controlling the security system over a local Wi-Fi network.

✨ Features

**Motion Detection:** Detects movement using a PIR sensor and triggers an alarm when the security system is armed.
**Web-Based Control:** Arms, disarms, and resets the alarm through a web interface accessible from a browser.
**Audible Alarm:** Activates a buzzer when motion is detected.
**Visual Alerts:** Flashes an LED during an alarm and keeps it on while the security system is armed.
**Physical Button Control:** Allows the user to arm or disarm the system and reset an active alarm using a physical button.
**Real-Time Status Monitoring:** Displays the current security status, PIR sensor state, buzzer status, and the latest system event.
**Automatic Wi-Fi Reconnection:** Periodically attempts to reconnect if the Wi-Fi connection is lost.
**PIR Warm-Up Period:** Waits 30 seconds after startup before enabling motion-triggered alarms.
**Non-Blocking Alarm Timing:** Uses millis() to control LED blinking without relying on blocking delays in the main loop.

**🛠 Components Used**
**Microcontroller:** ESP32 DevKit V1
**Motion Sensor:** PIR motion sensor (e.g., HC-SR501)
**Indicator:** LED with a current-limiting resistor
**Alarm:** Buzzer compatible with the ESP32 GPIO logic level
**Input:** Push button
**Connectivity:** Built-in Wi-Fi
**Software:** Arduino IDE, C++, WiFi.h, and WebServer.h
🔌 Wiring Instructions
1. Connecting the PIR Motion Sensor
**VCC** on the sensor ➔ to **3.3V**
**GND** on the sensor ➔ to **GND**
**OUT** on the sensor ➔ to **GPIO 27**
2. Connecting the LED
LED anode (+), through a current-limiting resistor ➔ **to GPIO 26**
LED cathode (−) ➔ to **GND**
3. Connecting the Buzzer
Buzzer control input (+) ➔ **to GPIO 25, if electrically compatible**
Buzzer **GND** (−) ➔ to **GND**

For buzzers that draw more current than an ESP32 GPIO can safely provide, use a suitable transistor driver.

4. Connecting the Physical Button
One button terminal ➔ **to GPIO 4**
The other button terminal ➔ **to GND**

The button uses the ESP32's internal pull-up resistor, configured with INPUT_PULLUP.

Note: Verify that all signals connected to ESP32 GPIO pins are compatible with its 3.3 V logic levels.

🚀 How to Run
1. Prerequisites

Make sure you have the following installed:

Arduino IDE
Your microcontroller board support installed through the Arduino Boards Manager

The project uses the following libraries:

WiFi.h
WebServer.h

*Both libraries are included with the standard ESP32 Arduino board package*

2. Configure Wi-Fi

Open the .ino file and update the Wi-Fi credentials:

const char* WIFI_SSID = "NAME WI-FI";
const char* WIFI_PASS = "PASSWORD Wi-FI";

AND

const char* AP_SSID   = "Name Wi-FI";
const char* AP_PASS   = "Password Wi-FI";


Replace Name WI-FI and Password WI-FI with your Wi-Fi network credentials.

Note: The ESP32 DevKit V1 typically connects to 2.4 GHz Wi-Fi networks.

3. Upload the Code
Connect the microcontroller to your computer using a USB cable.
Open the project in Arduino IDE.
Select the correct serial port.
Upload the code to the microcontroller.
Open the Serial Monitor and set the baud rate to 115200.
4. Access the Web Interface

After startup, the ESP32 attempts to connect to your Wi-Fi network. Once connected, its local IP address is printed in the Serial Monitor.

Open a browser on a device connected to the same local network and enter:

**http://YOUR__IP**

Replace YOUR__IP with the actual IP address displayed in the Serial Monitor.
The web interface provides three main controls:
Arm System: Activates security monitoring.
Disarm System: Disables security monitoring and clears an active alarm.
Reset Alarm: Clears the alarm and disarms the system.

**⚙️ How It Works**
**Startup:** The ESP32 initializes the GPIO pins, configures the PIR interrupt, and starts the web server.
**Wi-Fi Connection:** The device connects to the configured wireless network and prints its IP address.
**Sensor Warm-Up:** The system waits 30 seconds after startup before processing motion events for alarm activation.
**Armed Mode:** The LED stays on to indicate that the security system is armed.
**Motion Detection:** When the PIR sensor detects motion while the system is armed, the alarm is triggered.
**Alarm Mode:** The buzzer sounds continuously, and the LED flashes every 250 milliseconds.
**Alarm Reset:** The user can clear the alarm using the physical button or the web interface.
**Status Updates:** The web page refreshes automatically every 2 seconds to display the latest system status.

**⏱️ System Parameters**
Parameter	Value
PIR warm-up period	30 seconds
Button debounce interval	50 ms
Initial Wi-Fi connection timeout	15 seconds
Wi-Fi reconnection interval	10 seconds
LED alarm blink interval	250 ms
Web page refresh interval	2 seconds
Web server port	80

**This project is intended for educational and experimental purposes and is not a replacement for a certified security alarm system.**
