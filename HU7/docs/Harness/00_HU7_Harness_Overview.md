# HU7 Harness Overview

## CAN 연결

| Head Unit | SN65HVD230 |
|---|---|
| GPIO20 TX | RXD |
| GPIO19 RX | TXD |
| 3.3V | VCC |
| GND | GND |
| CANH | CANH Bus A |
| CANL | CANL Bus A |

- CAN Bus A는 Gateway CAN1 + SN65HVD230 측에 연결한다.
- CANH/CANL, 공통 GND, 종단저항 위치 확인.
- 화면·터치 패널 내부 연결은 Waveshare Vendor Driver가 관리한다.
