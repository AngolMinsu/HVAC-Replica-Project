# MKBD32

ESP32-S3 기반 MKBD 입력 노드.
기존 Arduino Uno R4 + MCP2515 구성을 ESP32-S3 내장 TWAI Controller와 SN65HVD230 CAN Transceiver 구성으로 전환했다.

## Pin Map

| Function | GPIO |
|---|---:|
| Driver Encoder CLK | 4 |
| Driver Encoder DT | 5 |
| Driver Encoder SW | 6 |
| Passenger Encoder CLK | 40 |
| Passenger Encoder DT | 41 |
| Passenger Encoder SW | 42 |
| DATC/INFO Button | 10 |
| Fan Up Button | 11 |
| Fan Down Button | 12 |
| Wind/Media Button | 13 |
| OLED SDA | 15 |
| OLED SCL | 16 |
| TWAI TX | 35 |
| TWAI RX | 36 |
| Fan PWM | 21 |

## CAN

MKBD32는 MCP2515를 사용하지 않는다.
ESP32-S3 내장 TWAI Controller와 SN65HVD230 CAN Transceiver를 사용한다.

```text
ESP32 GPIO35(TWAI TX) -> SN65HVD230 RXD
ESP32 GPIO36(TWAI RX) <- SN65HVD230 TXD
ESP32 3.3V            -> SN65HVD230 VCC
ESP32 GND             -> SN65HVD230 GND
```

단독 벤치에서는 `TWAI_MODE_NO_ACK`를 사용한다. 3노드 통합 시 `TWAI_MODE_NORMAL` 전환 여부를 검토한다.

## RTOS Task

```text
MKBD_CAN_RX   : 10ms, CAN 수신 처리
MKBD_INPUT    : 10ms, 버튼/엔코더 입력 처리
MKBD_OUTPUT   : 10ms, Fan PWM 출력 갱신
MKBD_DISPLAY  : 100ms, SH1106 OLED 갱신
```

Task 실행 진입점은 `task/MkbdRtos.cpp`다.
`MKBD32.ino`는 초기화 후 FreeRTOS Task를 시작하고 `loop()`은 idle 상태로 둔다.

## Module Boundary

- `app/MkbdHardware.*`: GPIO, 입력 sampling, OLED, Fan PWM
- `can/CanDriver.*`: ESP32 TWAI Driver
- `can/MkbdCanService.*`: CAN Frame 송수신과 상태 Broadcast
- `app/AppLogic.*`: 입력 Event에 따른 상태 변경 규칙
- `task/`: 주기별 실행 흐름
