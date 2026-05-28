# Zarduino Car Controller

**Live Demo:** [zarduino.netlify.app](https://zarduino.netlify.app) *(update after deploy)*

A browser-based BLE controller for an Arduino-powered car. Uses Web Bluetooth to connect to an ESP32 bridge, which relays commands to an Arduino motor controller via serial.

## System Architecture

```
Web Browser  -->  ESP32 (BLE Server)  -->  Arduino (Serial)  -->  Motors + Servo
```

1. **Web App** (`index.html`) -- sends commands (`f`, `b`, `l`, `r`, `s`) via Web Bluetooth API
2. **ESP32 Bridge** (`firmware/ble_bridge.ino`) -- receives BLE data and forwards over UART
3. **Arduino Car** (`firmware/car_controller.ino`) -- drives two DC motors and a steering servo

## Web Controller Features

- Manual D-pad buttons (click or touch)
- Keyboard support: arrow keys or WASD
- Joystick mode with Xbox gamepad support (left stick)
- Dark/Light theme toggle
- Real-time BLE connection status

## Hardware Wiring

| Connection | Detail |
|---|---|
| ESP32 TX to Arduino RX | GPIO 17 to Arduino Pin 12 |
| Steering Servo | Arduino Pin 8 |
| Left Motor | Arduino Pins 3 and 11 |
| Right Motor | Arduino Pins 5 and 6 |

## BLE Configuration

- Device Name: `Arduino-BLE-Bridge`
- Service UUID: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- Characteristic UUID: `beb5483e-36e1-4688-b7f5-ea07361b26a8`

## Firmware Setup

1. Flash `firmware/ble_bridge.ino` to the ESP32 using Arduino IDE (install ESP32 board support)
2. Flash `firmware/car_controller.ino` to the Arduino
3. Wire according to the table above

## Web App Deployment

> Web Bluetooth API requires **HTTPS** and Chrome or Edge.

Deploy to Netlify:
1. Connect this repo to [Netlify](https://app.netlify.com)
2. Publish directory: `.` (root)
3. No build command needed

## Commands

| Command | Action |
|---|---|
| `f` | Forward |
| `b` | Backward |
| `l` | Turn Left |
| `r` | Turn Right |
| `s` | Stop |

## Servo Trim

Adjust `initAngle` in `car_controller.ino` to center your steering. Default is 75 degrees.

## License
MIT License
