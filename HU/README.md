# HU Head Unit

HVAC Replica 프로젝트의 헤드 유닛 입니다.

## 개요

`HU`는 ESP8266을 Head Unit 역할로 사용하기 위한 테스트 노드이다.

MKBD 유닛이 실제 버튼 입력과 OLED 표시를 담당한다면, HU 유닛은 시리얼 명령을 받아 CAN 프레임을 생성하고 MKBD로 제어 요청을 보낸다. MKBD가 응답하거나 상태를 Broadcast하면 HU는 해당 프레임을 Serial Monitor에 출력한다.

```text
PC Serial Monitor
  -> ESP8266 HU
  -> MCP2515 CAN
  -> MKBD
```

## 역할

- Serial Monitor에서 사용자 명령 입력
- 명령을 CAN Payload로 변환
- CAN ID `0x100`으로 MKBD에 제어 요청 송신
- MKBD의 응답 CAN ID `0x101` 수신
- MKBD 상태 Broadcast CAN ID `0x300` 수신
- RX/TX 프레임과 Payload 요약을 Serial Monitor에 출력

## 하드웨어

사용 보드:

- ESP8266 Lolin D2 Mini
- MCP2515 CAN Module
- TJA1050 CAN Transceiver

기본 CAN CS 핀은 `GDS.h`에 정의되어 있다.

```cpp
const uint8_t GDS_PIN_CAN_CS = D8;
```

MCP2515 모듈 배선에 따라 CS 핀은 변경할 수 있다. ESP8266에서 `D8`은 부팅 스트랩 핀이므로, 업로드나 부팅 문제가 있으면 `D2` 같은 다른 GPIO로 변경하는 것이 좋다.

## Serial 설정

Serial Monitor baud rate:

```text
115200
```

줄 끝 설정은 `Newline` 또는 `Both NL & CR`을 사용하면 된다.

## CAN 설정

| 항목           | 값              |
| -------------- | --------------- |
| CAN Type       | Standard 11-bit |
| CAN Speed      | 500 kbps        |
| DLC            | 8 bytes         |
| HU -> MKBD     | `0x100`         |
| MKBD -> HU     | `0x101`         |
| MKBD Broadcast | `0x300`         |

## Payload 구조

HU와 MKBD는 8바이트 고정 Payload를 사용한다.

| Byte  | 의미     |
| ----- | -------- |
| Byte0 | Service  |
| Byte1 | Result   |
| Byte2 | Signal   |
| Byte3 | Value    |
| Byte4 | Option   |
| Byte5 | Reserved |
| Byte6 | Counter  |
| Byte7 | Checksum |

Checksum은 Byte0부터 Byte6까지 XOR한 값이다.

## 지원 명령어

### HVAC 제어

```text
fan 0..8
temp 18..30
wind face|foot|def|mix
power on|off
ac on|off
auto on|off
```

### 화면 및 INFO 제어

```text
screen datc|info
media ready|dev
volume 0..30
vol 0..30
map ready|dev
```

### 상태 읽기

```text
read power
read fan
read temp
read wind
read ac
read auto
read screen
read media
read volume
read map
```

도움말:

```text
help
?
```

## 사용 예시

INFO 화면으로 전환:

```text
screen info
```

볼륨 변경:

```text
volume 15
```

팬 속도 변경:

```text
fan 3
```

현재 화면 상태 읽기:

```text
read screen
```

## 로그 예시

HU에서 명령을 입력하면 TX 프레임이 먼저 출력된다.

```text
CAN TX ID:0x100 DLC:8 DATA: 20 00 08 01 00 00 00 29
  Service:WRITE_REQ Signal:SCREEN Value:1 Result:NORMAL
```

MKBD가 요청을 처리하면 응답 또는 Broadcast가 RX로 출력된다.

```text
CAN RX ID:0x101 DLC:8 DATA: 22 02 08 01 00 00 00 29
  Service:WRITE_RES Signal:SCREEN Value:1 Result:SUCCESS

CAN RX ID:0x300 DLC:8 DATA: 32 00 08 01 00 00 03 38
  Service:READ_RES Signal:SCREEN Value:1 Result:NORMAL
```

## 파일 구성

현재 HU 코드는 다음 파일로 구성된다.

```text
HU.ino
GDS.h
CanDriver.h / CanDriver.cpp
CanProtocol.h / CanProtocol.cpp
CanMonitor.h / CanMonitor.cpp
CommandParser.h / CommandParser.cpp
```

역할:

| 파일            | 역할                                             |
| --------------- | ------------------------------------------------ |
| `HU.ino`        | ESP8266 setup/loop, Serial 입력, CAN 송수신 조립 |
| `GDS.h`         | CAN ID, Service, Signal, 핀, 범위 상수           |
| `CanDriver`     | MCP2515 드라이버 계층                            |
| `CanProtocol`   | 8바이트 Payload 생성/변환/Checksum               |
| `CanMonitor`    | Serial Monitor용 CAN 로그 출력                   |
| `CommandParser` | Serial 명령어를 CAN 요청으로 변환                |

## 설계 의도

HU는 실제 차량 Head Unit을 완전히 구현하기보다, MKBD 유닛의 CAN 제어 기능을 검증하기 위한 테스트 송신기 역할을 한다.

따라서 현재 버전은 웹 UI나 버튼 UI 없이 Serial Monitor 명령 기반으로 동작한다. 이 구조만으로도 MKBD가 Head Unit에서 온 CAN 요청을 수신하고, 상태를 변경하고, 응답과 Broadcast를 보내는 전체 흐름을 확인할 수 있다.
