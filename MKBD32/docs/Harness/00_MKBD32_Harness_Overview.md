# MKBD32 Harness Overview

## GPIO

| 기능 | GPIO |
|---|---:|
| Driver Encoder A / B / SW | 4 / 5 / 6 |
| Passenger Encoder A / B / SW | 40 / 41 / 42 |
| Screen Button | 10 |
| Fan Up / Down | 11 / 12 |
| Wind / Media | 13 |
| OLED SDA / SCL | 15 / 16 |
| TWAI TX / RX | 35 / 36 |
| Fan PWM | 21 |

## CAN 연결

| MKBD32 | SN65HVD230 |
|---|---|
| GPIO35 TX | RXD |
| GPIO36 RX | TXD |
| 3.3V | VCC |
| GND | GND |
| CANH | CANH Bus B |
| CANL | CANL Bus B |

- CAN Bus B는 Gateway MCP2515 + TJA1050 측에 연결한다.
- Enc A/B/SW는 `INPUT_PULLUP` 기준. GND 공통 연결.
