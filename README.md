# Zarduino Car Controller

**Live Demo:** [zarduino.netlify.app](https://zarduino.netlify.app)

A browser-based BLE controller for an Arduino-powered car. Uses Web Bluetooth to connect to an ESP32 bridge, which relays commands to an Arduino motor controller via serial.

## System Architecture

```
Web Browser  -->  ESP32 (BLE Server)  -->  Arduino (Serial)  -->  Motors + Servo
```

1. **Web App** (`index.html`) -- sends commands (`f`, `b`, `l`, `r`, `s`) via Web Bluetooth API
2. **ESP32 Bridge** (`firmware/ble_bridge.ino`) -- receives BLE data and forwards over UART
3. **Arduino Car** (`firmware/car_controller.ino`) -- drives two DC motors and a steering servo

## Web Controller Features

- Manual D-pad buttons (click or touch) with a speed slider
- Keyboard support: arrow keys or WASD (releasing one key falls back to another key still held)
- Analog joystick mode with Xbox gamepad support (left stick): speed and steering follow the stick
- Dark/Light theme toggle
- Real-time BLE connection status

## Safety

- **Watchdog:** while the car moves, the page repeats the current command every 150 ms. If the Arduino hears nothing for 500 ms (link lost, tab closed, bridge reset) it stops the motors and centers the steering.
- **Stop on disconnect:** the ESP32 sends a stop to the Arduino as soon as the BLE client disconnects.
- **Stop when hidden:** switching tabs or locking the phone sends a stop.
- **One write at a time:** the page never starts a BLE write while another is pending; the newest command is sent next.
- **Optional pairing:** set `REQUIRE_PASSKEY` to 1 in `ble_bridge.ino` and choose your own `PASSKEY`. The operating system then asks for the passkey before the page can drive the car.

Flash both firmware files after updating; the new page and the new car firmware go together.

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
| `J<speed>,<steer>` + newline | Analog drive: speed -100..100 (negative is reverse), steer -100..100 (negative is left), in percent |

Single-letter commands drive at full speed. The page sends `J` commands for driving and `s` for stop.

## Servo Trim

Adjust `initAngle` in `car_controller.ino` to center your steering. Default is 75 degrees.

## License
MIT License
