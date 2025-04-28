# Weather Station with ESP8266 and OLED Display

This project is a weather station built using an ESP8266 microcontroller with a 0.96" OLED display. It includes a local configuration website for easy setup and customization.

---

## Features
- Real-time weather data display on the OLED screen.
- Local web interface for configuration.
- Portable and battery-powered design.

---

## Hardware Requirements
- **ESP8266** with onboard 0.96" OLED display.
- **TP4056** charging module.
- **Lithium-ion battery** for portable power.

---

## Software Requirements
- **Arduino IDE** (or any compatible IDE).
- Libraries:
  - **Adafruit SSD1306** for OLED display.
  - **ESP8266WiFi** for WiFi connectivity.
  - Include other libraries as applicable.

---

## Setup Instructions

### Hardware Setup
1. Connect the ESP8266 to the OLED display (if not already onboard).
2. Wire the lithium-ion battery to the TP4056 charging module.
3. Connect additional components as needed.

### Software Setup
1. Install the required libraries in the Arduino IDE:
   - Go to `Sketch > Include Library > Manage Libraries`.
   - Search for and install the required libraries.
2. Clone this repository:
   ```bash
   git clone https://github.com/drngominhtu/Weather_Station_DR.git
