# HU7 Project Context

## 역할

Waveshare ESP32-S3 Touch LCD 7B 기반 차량 Head Unit.

- 1024x600 Touch LCD
- LVGL + SquareLine Studio UI
- ESP32-S3 TWAI + SN65HVD230 CAN Bus A
- MKBD 상태 수신 및 HMI 반영

## 핵심 파일

| 경로 | 역할 |
|---|---|
| `HU7.ino` | 시작점 |
| `GDS.h` | GPIO, CAN, Task 상수 |
| `src/can/` | TWAI 송수신, Payload, 로그 |
| `src/hmi/` | HVAC/HMI 상태와 UI 연결 |
| `src/task/` | CAN RX 10ms, Input 10ms, UI 20ms |
| `SSUI/` | SquareLine Studio 원본 |
| `src/generated/squareline/` | SquareLine 생성 코드 |
| `src/vendor/waveshare_7b/` | Waveshare 보드 드라이버 |

## CAN

- TX: GPIO20
- RX: GPIO19
- CAN Select: IO Extension GPIO5
- 500 kbps, Standard ID, DLC 8
- 상세: `docs/Harness/02_HU7_CAN_Protocol.md`

## 현재 작업

- Gateway ACK 수신 처리 연결
- MKBD 상태를 HVAC/INFO 화면에 반영
- SquareLine UI 화면 전환과 Back 흐름 정리
