# STM32 CAN Bus Sensor Network

## Project Overview
This project implements a distributed sensor network using two STM32F103C8T6 microcontrollers communicating over CAN bus. The sensor node collects real-time data from multiple sensors and broadcasts it over CAN, while the display node receives, processes, and visualizes the data on an LCD with PWM-controlled LED brightness.

**Sensor Node** - Data acquisition and transmission:
- Reads DHT11 temperature and humidity every second
- Reads MPU6050 accelerometer and gyroscope at 50ms intervals
- Reads DS3231 real-time clock for timestamps
- Reads potentiometer via ADC for LED brightness control
- Handles button press to cycle display modes on the remote node
- Transmits all data over CAN bus using 6 message IDs

**Display Node** - Data reception and visualization:
- Receives CAN messages via hardware filtering and interrupts
- Displays sensor data on 16x2 I2C LCD across 4 switchable views
- Controls LED brightness via PWM based on potentiometer data
- Responds to remote CAN commands for view switching

**CAN Protocol** – Custom 6-ID message format at 500 kbps:
- Hardware filtering accepts messages on IDs 0x100 through 0x107
- Interrupt-driven reception for immediate data processing
- Bi-directional communication: sensor data one way, display commands the other

The system demonstrates real-world CAN bus communication between embedded nodes with multiple sensors, remote display control, and efficient message handling.

## Video Demonstration

## Hardware
**Sensor Node**
- STM32F103C8T6 (Blue Pill)
- DHT11 Temperature & Humidity Sensor
- MPU6050 6-Axis Accelerometer & Gyroscope
- DS3231 Real-Time Clock Module
- 10kΩ Potentiometer (ADC input)
- Push Button (display view switcher)
- CAN Transceiver (SN65HVD230 or TJA1050)

**Display Node**
- STM32F103C8T6 (Blue Pill)
- 16x2 I2C LCD Display (PCF8574 backpack)
- LED with resistor (PWM brightness)
- CAN Transceiver (SN65HVD230 or TJA1050)

## Project Schematic
### Sensor Node
<img width="986" height="536" alt="Schematic Diagram Sensor Node" src="https://github.com/user-attachments/assets/7a8b80c3-707a-4f8c-8e25-e38a49c8d596" />

### Display Node
<img width="1166" height="387" alt="Schematic Diagram Display Node" src="https://github.com/user-attachments/assets/1e4d24e5-e010-42eb-8eaa-aa2206e4754a" />

## Pin Configuration
### Sensor Node
| Component | STM32 Pin | Notes |
|-----------|-----------|-------|
| Potentiometer | PA0 | ADC input, 0-4095 range |
| Button (Mode Switch) | PA1 | External pull-up, cycles display views via CAN |
| DHT11	| PB0 | Single-wire data |
| MPU6050 | PB8 (SCL), PB9 (SDA) | I2C1, address: 0x69 |
| DS3231 | PB8 (SCL), PB9 (SDA) | I2C1, address: 0x68 |
| CAN Transceiver | PA12 (TX), PA11 (RX) | SN65HVD230 or TJA1050 |
| USART1 (Debug) | PA9 (TX), PA10 (RX) | 115200 baud, 8N1 |

### Display Node
| Component | STM32 Pin | Notes |
|-----------|-----------|-------|
| LED (PWM) | PA8 | TIM1_CH1, brightness controlled by potentiometer data |
| LCD (I2C) | PB10 (SCL), PB11 (SDA) | I2C2, PCF8574 backpack, address: 0x4E |
| CAN Transceiver | PA12 (TX), PA11 (RX) | SN65HVD230 or TJA1050 |
| USART1 (Debug) | PA9 (TX), PA10 (RX) | 115200 baud, 8N1 |

## CAN Bus
### CAN Bus Connection
| Connection | Notes |
|------------|-------|
| CANH to CANH | Twisted pair wiring |
| CANL to CANL | Twisted pair wiring |
| 120Ω Termination | Built-in on SN65HVD230 module (both ends) |

### CAN Protocol
| CAN ID | Bytes | Data | Update Rate |
|--------|-------|------|-------------|
| 0x100 | 2 | Potentiometer value (0-4095) | 10ms |
| 0x101 | 4 | Temperature & Humidity (fixed-point) | 1s |
| 0x102 | 6	| Accelerometer X, Y, Z (raw) | 50ms |
| 0x103 | 6	| Gyroscope X, Y, Z (raw) | 50ms |
| 0x105 | 6	| Time & Date (BCD from DS3231) | 1s |
| 0x106 | 1	| Display view command (0-3) | On button press |
- Baud Rate: 500 kbps
- Hardware Filter: 32-bit mask mode, Filter Bank 0

Instead of receiving every CAN message and checking IDs in software, the STM32's CAN peripheral can filter messages in hardware. Messages that don't pass the filter are discarded before they reach the CPU, saving processing time.

### Filter Configuration
```
Base ID:  0x100  =  0001 0000 0000  (11 bits)
Mask:     0x7F8  =  0111 1111 1000  (11 bits)

How masking works:
Bit position:  10  9  8  7  6  5  4  3  2  1  0
Base (0x100):   0  0  0  1  0  0  0  0  0  0  0
Mask (0x7F8):   0  1  1  1  1  1  1  1  1  0  0

Mask bit = 0 → Must match base ID (fixed)
Mask bit = 1 → Can be 0 or 1 (wildcard)
```

The mask 0x7F8 in binary is 0111 1111 1000:
- Bits 10: Must be 0 (fixed)
- Bits 9-3: Can be anything (wildcard)
- Bits 2-0: Must be 000 (fixed)

This creates a range:
```
Base:   0x100 = 0001 0000 0000
Range:  0x100 = 0001 0000 0000
        0x101 = 0001 0000 0001
        0x102 = 0001 0000 0010
        0x103 = 0001 0000 0011
        0x104 = 0001 0000 0100
        0x105 = 0001 0000 0101
        0x106 = 0001 0000 0110
        0x107 = 0001 0000 0111
        Accepts all 8 IDs
```

## Display Views
A button on the sensor node sends CAN ID 0x106 to cycle through display modes remotely:

| Mode | Line 1 | Line 2 |
|------|--------|--------|
| 0 | TEMP: 25.5 C | HUMD: 60.0 % |
| 1 | TIME: 14:30:25 | DATE: 17/05/2026 |
| 2 | AX:0.98 AY:0.02 | AZ:-0.04 [g] |
| 3	| GX:0.12 GY:-0.03 | GZ:0.01 [dps] |

### Data Format

| Sensor | Raw Format | Conversion | Example |
|--------|------------|------------|---------|
| Temperature | 16-bit, little-endian | value/10 = °C | 255 → 25.5°C |
| Humidity | 16-bit, little-endian | value/10 = % | 600 → 60.0% |
| Accelerometer | 3×16-bit, big-endian | raw/16384 = g | 8192 → 0.50g |
| Gyroscope | 3×16-bit, big-endian | raw/131 = °/s | 65 → 0.5°/s |
| Time/Date | BCD encoded | BCD to decimal | 0x14 → 14 |

## Related Projects 
- [STM32_MicroSD_Cloud_Logger](https://github.com/rubin-khadka/STM32_MicroSD_Cloud_Logger)
- [STM32_CAN_Communication](https://github.com/rubin-khadka/STM32_CAN_Communication)
- [STM32_OTA_Bootloader_W5500](https://github.com/rubin-khadka/STM32_OTA_Bootloader_W5500)

## Resources
- [STM32F103 Datasheet](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf)
- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [DHT11 Sensor Datasheet](https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf)
- [MPU6050 Sensor Datasheet](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf)
- [RTC DS3231 Datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/ds3231.pdf)
- [PCF8574 I2C Backpack Datasheet](https://www.ti.com/lit/ds/symlink/pcf8574.pdf)
- [STM32 CAN(bxCAN)](https://community.st.com/t5/stm32-mcus/using-can-bxcan-in-normal-mode-with-stm32-microcontrollers-part/ta-p/800502)
- [TJA1050 Datasheet](https://www.nxp.com/docs/en/data-sheet/TJA1050.pdf)
- [SN65HVD230 Datasheet](https://www.ti.com/lit/ds/symlink/sn65hvd232.pdf?ts=1779165701894)

## Project Status
- **Status**: Complete
- **Version**: v1.0
- **Last Updated**: May 2026

## Contact
**Rubin Khadka Chhetri**  
📧 rubinkhadka84@gmail.com <br>
🐙 GitHub: https://github.com/rubin-khadka
