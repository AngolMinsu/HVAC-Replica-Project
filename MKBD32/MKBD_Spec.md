# MKBD Spec

## 1. 개요

MKBD는 Arduino Uno R4 Minima 기반의 전환형 키보드 유닛이다.

하나의 버튼 패널을 사용하지만, 현재 화면 모드에 따라 버튼의 역할이 달라진다.

```text
DATC 화면
  -> HVAC 제어

INFO 화면
  -> Media / Volume / Map 제어
```

MKBD는 로컬 버튼 입력과 Head Unit에서 들어오는 CAN 제어 요청을 모두 처리한다. 상태가 변경되면 OLED 화면, LED, Fan PWM을 갱신하고 변경된 CAN Signal을 Broadcast한다.

## 2. 하드웨어 구조

| 구성 | 역할 |
| --- | --- |
| Arduino Uno R4 Minima | 메인 제어 보드 |
| SH1106 OLED | DATC / INFO 화면 표시 |
| Push Button | 화면 전환 및 기능 입력 |
| Status LED | HVAC 동작 상태 표시 |
| 5V DC Fan | Fan Speed 출력 확인 |
| 2N2222A Transistor | Fan PWM 구동 |
| MCP2515 CAN Module | CAN Controller |
| TJA1050 | CAN Transceiver |

## 3. 핀 구성

| 기능 | Pin |
| --- | --- |
| Button 2 | `D2` |
| Button 3 | `D3` |
| Button 4 | `D4` |
| Button 5 | `D5` |
| Screen Button | `D6` |
| Status LED | `D7` |
| Fan Motor PWM | `D9` |
| MCP2515 CS | `D10` |
| OLED I2C | `SDA / SCL` |

버튼은 `INPUT_PULLUP` 방식으로 사용한다.

## 4. 소프트웨어 구조

```text
MKBD/
  MKBD.ino
  GDS.h

  state/
  button/
  app/
  display/
  can/
```

| 경로 | 역할 |
| --- | --- |
| `MKBD.ino` | Arduino setup/loop, 전체 흐름 조립 |
| `GDS.h` | 핀 번호, 범위, CAN ID, Signal 상수 |
| `state/` | MKBD 전체 상태 관리 |
| `button/` | 버튼 edge 감지와 debounce 처리 |
| `app/` | 버튼 이벤트를 상태 변경 로직에 연결 |
| `display/` | DATC / INFO 화면 출력 및 화면별 버튼 동작 |
| `can/` | CAN 송수신, Payload 처리, 로그 출력 |

## 5. 상태 구조

MKBD의 주요 상태는 `SystemState`에 모아 관리한다.

```text
screenMode
hvacMode
fanSpeed
setTemp
windMode
volume
mediaReady
mapReady
```

로컬 버튼 입력과 CAN 요청은 모두 이 상태를 변경하는 방식으로 처리된다.

## 6. 화면별 버튼 역할

DATC 화면:

| 버튼 | 역할 |
| --- | --- |
| `D2` | HVAC Mode 변경 |
| `D3` | Fan Speed 변경 |
| `D4` | Temperature 변경 |
| `D5` | Wind Mode 변경 |

INFO 화면:

| 버튼 | 역할 |
| --- | --- |
| `D2` | Media 상태 변경 |
| `D3` | Volume 증가 |
| `D4` | Volume 감소 |
| `D5` | Map 상태 변경 |

Screen Button `D6`은 DATC / INFO 화면을 전환한다.

## 7. CAN 구조

MKBD는 MCP2515 모듈을 통해 CAN 통신을 수행한다.

| CAN ID | 방향 | 의미 |
| --- | --- | --- |
| `0x100` | HU -> MKBD | Control Request |
| `0x101` | MKBD -> HU | Control Response |
| `0x300` | MKBD -> ALL | Status Broadcast |

CAN Payload 규격은 `CanSpec.md`를 따른다.

MKBD의 CAN 처리 흐름:

```text
CAN Frame 수신
  -> Payload 해석
  -> 요청 검증
  -> SystemState 변경
  -> Response 송신
  -> 변경된 Signal Broadcast
```

## 8. 설계 특징

- 하드웨어 제어와 Payload 처리 로직을 분리했다.
- MCP2515 송수신은 `CanDriver`가 담당한다.
- CAN Payload 해석은 `CanProtocol`, `CanHandler`가 담당한다.
- 버튼 입력, 상태 변경, 화면 출력, CAN 처리를 모듈별로 분리했다.
- 단위 테스트는 하드웨어 의존성이 낮은 로직 계층을 대상으로 할 수 있다.

## 9. 한 줄 요약

MKBD는 DATC/INFO 화면에 따라 버튼 역할이 바뀌는 Arduino 기반 전환형 키보드 유닛이며, 로컬 버튼 입력과 CAN 제어 요청을 같은 상태 구조로 처리하도록 설계했다.
