# MKBD32

ESP32-S3 기반 MKBD 입력 노드.
기존 Arduino Uno R4 + MCP2515 구조를 ESP32-S3 + TWAI + SN65HVD230 구조로 이식한 버전이다.

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

## RTOS Task

```text
MKBD_CAN_RX   : 10ms, CAN 수신 처리
MKBD_INPUT    : 10ms, 버튼/엔코더 입력 처리
MKBD_OUTPUT   : 10ms, Fan PWM 출력 갱신
MKBD_DISPLAY  : 100ms, SH1106 OLED 갱신
```

Task 실행 진입점은 `task/MkbdRtos.cpp`이다.
`MKBD32.ino`는 초기화 후 FreeRTOS Task를 시작하고, `loop()`는 idle 상태로 둔다.