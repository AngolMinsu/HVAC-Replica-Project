# MKBD32 Project Context

## 역할

ESP32-S3 기반 차량 MKBD 입력 노드.

- Driver / Passenger 로터리 엔코더
- DATC / INFO 전환 버튼
- SH1106 OLED, Fan PWM 출력
- ESP32-S3 TWAI + SN65HVD230 CAN Bus B
- Gateway를 통해 Head Unit과 상태 공유

## 핵심 파일

| 경로 | 역할 |
|---|---|
| `MKBD32.ino` | 시작점 |
| `GDS.h` | GPIO, CAN, 상태 범위, PWM 상수 |
| `MkbRtos.*` | Task 생성과 주기 |
| `app/` | DATC/INFO 상태 전이 |
| `button/`, `encoder/` | 물리 입력 처리 |
| `can/` | TWAI, Payload, 수신 처리 |
| `display/` | OLED DATC/INFO 출력 |
| `task/` | 10ms Input/CAN/Output, 100ms Display |

## 현재 작업

- 엔코더 회전 입력 하드웨어 안정화
- 3노드 통합 운용을 위해 `TWAI_MODE_NORMAL` 적용 및 실제 Bus ACK 검증
- Gateway ACK와 반대 노드 Response 처리 연결
