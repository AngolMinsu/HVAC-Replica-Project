# MKBD CAN 통신 명세서

## 1. 개요

본 프로젝트는 차량 공조 시스템(HVAC) 조작부인 MKBD와 Head Unit 사이의 CAN 통신 구조를 모사한다.

MKBD는 Arduino Uno R4 Minima 기반으로 구성되며, 버튼 입력과 OLED UI 표시, 팬 제어, CAN 송수신을 담당한다.

Head Unit은 ESP8266 Lolin D2 Mini 기반으로 구성되며, Web UI와 MKBD 제어 요청 송신, 상태 데이터 수신을 담당한다.

CAN 통신은 CAN 2.0A Standard Frame을 사용한다.

---

## 2. 노드 정의

### Node 1: MKBD

보드:

```text
Arduino Uno R4 Minima
```

역할:

- DATC/HVAC 버튼 입력 처리
- SH1106 OLED UI 표시
- 팬 PWM 제어
- CAN 프레임 송수신
- HVAC 상태 관리
- Head Unit 요청 처리

---

### Node 2: Head Unit

보드:

```text
ESP8266 Lolin D2 Mini
```

역할:

- Web UI 처리
- MKBD 제어 요청 송신
- MKBD 응답 수신
- HVAC 상태 데이터 수신

---

## 3. CAN 물리 계층

| 항목 | 내용 |
| --- | --- |
| Protocol | CAN 2.0A |
| Frame Type | Standard Frame |
| Identifier Length | 11-bit |
| Payload Length | 8 Byte fixed |
| Bit Rate | 500 kbps |
| CAN Controller | MCP2515 |
| CAN Transceiver | TJA1050 |
| Arduino Library | `mcp_can` |
| Communication Interface | SPI |

MCP2515는 CAN 프레임의 SOF, Arbitration, CRC, ACK, EOF 등 하위 CAN 프레임 처리를 담당한다.

프로젝트 코드는 MCP2515가 전달한 CAN ID와 8바이트 Payload만 해석한다.

---

## 4. CAN 프레임 구조

CAN 2.0A Standard Frame 구조는 다음과 같다.

```text
| SOF | Identifier | RTR | IDE | r0 | DLC | DATA | CRC | ACK | EOF |
```

| Field | 설명 |
| --- | --- |
| SOF | Start Of Frame |
| Identifier | 11-bit CAN ID |
| RTR | Remote Transmission Request |
| IDE | Identifier Extension |
| r0 | Reserved Bit |
| DLC | Data Length Code |
| DATA | Payload Data |
| CRC | Error Check |
| ACK | Acknowledge |
| EOF | End Of Frame |

중요한 점:

```text
CAN Identifier는 Payload에 포함되지 않는다.
```

예시:

```text
CAN ID  : 0x100
Payload : 20 00 02 03 00 00 01 20
```

위 예시에서 `0x100`은 CAN 프레임의 Identifier 필드이고, Payload 8바이트에는 포함되지 않는다.

---

## 5. CAN Identifier 정의

현재 구현은 Head Unit이 MKBD에 제어 요청을 보내고, MKBD가 응답하는 구조를 기준으로 한다.

| CAN ID | Sender | Receiver | 설명 |
| --- | --- | --- | --- |
| `0x100` | HU | MKBD | Control Request |
| `0x101` | MKBD | HU | Control Response |
| `0x300` | MKBD | ALL | HVAC Status Broadcast |

참고:

기존 초안에는 `0x100`과 `0x101`의 방향이 표와 예시에서 다르게 표현된 부분이 있었다.

본 명세서는 Fan Speed Write Request 예시와 현재 코드 구현을 기준으로 방향을 확정한다.

```text
0x100 = HU -> MKBD
0x101 = MKBD -> HU
0x300 = MKBD -> ALL
```

---

## 6. Payload 구조

모든 프로젝트 CAN Payload는 8바이트 고정 길이를 사용한다.

| Byte | Field | 설명 |
| --- | --- | --- |
| Byte0 | Service | 요청/응답 종류 |
| Byte1 | Result | 응답 결과 |
| Byte2 | Signal | 제어 또는 조회 대상 |
| Byte3 | Value | 설정값 또는 응답값 |
| Byte4 | Option | 추가 옵션 또는 Error Code |
| Byte5 | Reserved | 예약 영역, 현재 0 |
| Byte6 | Counter | Alive Counter |
| Byte7 | Checksum | XOR Checksum |

---

## 7. Service 정의

| 값 | 이름 | 설명 |
| --- | --- | --- |
| `0x20` | Write Request | HU가 MKBD 상태 변경을 요청 |
| `0x22` | Write Response | MKBD가 Write Request 결과 응답 |
| `0x30` | Read Request | HU가 MKBD 상태 조회 요청 |
| `0x32` | Read Response | MKBD가 Read Request 결과 응답 |

---

## 8. Result 정의

| 값 | 이름 | 설명 |
| --- | --- | --- |
| `0x00` | Normal | 일반 상태 또는 Broadcast |
| `0x02` | Success | 요청 처리 성공 |
| `0x0F` | Fail | 요청 처리 실패 |

---

## 9. Error Code 정의

`Result = 0x0F`인 실패 응답에서는 Byte4 `Option` 필드를 Error Code로 사용한다.

| 값 | 이름 | 설명 |
| --- | --- | --- |
| `0x00` | No Error | 오류 없음 |
| `0x01` | Checksum Error | Checksum 불일치 |
| `0x02` | Unsupported Service | 지원하지 않는 Service |
| `0x03` | Unsupported Signal | 지원하지 않는 Signal |
| `0x04` | Value Out Of Range | Value 범위 오류 |
| `0x05` | Invalid DLC | DLC 오류 |
| `0x06` | Counter Error | Alive Counter 오류 |
| `0x07` | Driver Error | MCP2515 또는 송수신 드라이버 오류 |

현재 구현 정책:

- Checksum 오류는 Fail Response를 송신한다.
- Unsupported Service는 Fail Response를 송신한다.
- Unsupported Signal은 Fail Response를 송신한다.
- Value Out Of Range는 Fail Response를 송신한다.
- Invalid DLC는 Payload 필드를 신뢰할 수 없으므로 Serial 로그만 남기고 응답하지 않는다.
- Driver Error는 상대 노드에 응답하기 어려우므로 로컬 Serial 진단 로그로 처리한다.
- Counter Error는 확장용으로 정의되어 있으며, 현재는 순서 검증을 수행하지 않는다.

---

## 10. Signal 정의

| 값 | Signal | 설명 |
| --- | --- | --- |
| `0x01` | Power | HVAC 전원 상태 |
| `0x02` | Fan Speed | 팬 속도 |
| `0x03` | Temperature | 설정 온도 |
| `0x04` | Mode | 풍향 모드 |
| `0x05` | A/C | A/C 상태 |
| `0x06` | Auto | AUTO 모드 |
| `0x07` | Intake | 내기/외기 전환 |
| `0x08` | Screen Mode | MKBD 화면/키맵 모드 전환 |
| `0x09` | Media | INFO 화면 Media 상태 |
| `0x0A` | Volume | INFO 화면 Volume 값 |
| `0x0B` | Map | INFO 화면 Map 상태 |

---

## 11. Signal 값 사양

### Power

| 값 | 설명 |
| --- | --- |
| `0x00` | OFF |
| `0x01` | ON |

---

### Fan Speed

| 값 | 설명 |
| --- | --- |
| `0x00` | Level 0 |
| `0x01` | Level 1 |
| `0x02` | Level 2 |
| `0x03` | Level 3 |
| `0x04` | Level 4 |
| `0x05` | Level 5 |
| `0x06` | Level 6 |
| `0x07` | Level 7 |
| `0x08` | Level 8 |

허용 범위:

```text
0x00 ~ 0x08
```

---

### Temperature

Temperature 값은 섭씨 온도 값을 그대로 1바이트에 넣는다.

| 값 | 설명 |
| --- | --- |
| `0x12` | 18°C |
| `0x13` | 19°C |
| `0x14` | 20°C |
| `0x15` | 21°C |
| `0x16` | 22°C |
| `0x17` | 23°C |
| `0x18` | 24°C |
| `0x19` | 25°C |
| `0x1A` | 26°C |
| `0x1B` | 27°C |
| `0x1C` | 28°C |
| `0x1D` | 29°C |
| `0x1E` | 30°C |

허용 범위:

```text
0x12 ~ 0x1E
```

현재 코드의 `state.setTemp`는 10진수 `18~30`을 사용한다.
CAN Payload에 들어갈 때는 동일한 숫자가 1바이트 hex 값으로 표현된다.

예:

```text
18 decimal = 0x12
30 decimal = 0x1E
```

---

### Mode

`Mode`는 현재 구현에서 풍향 모드(`state.windMode`)로 매핑한다.

| 값 | 설명 | 코드 매핑 |
| --- | --- | --- |
| `0x00` | Face | `WIND_FACE` |
| `0x01` | Foot | `WIND_FOOT` |
| `0x02` | Defrost | `WIND_DEF` |
| `0x03` | Face + Foot | `WIND_MIX` |

---

### A/C

| 값 | 설명 |
| --- | --- |
| `0x00` | A/C OFF |
| `0x01` | A/C ON |

현재 구현에서 `0x01` 요청은 `state.hvacMode = HVAC_AC`로 반영된다.

---

### Auto

| 값 | 설명 |
| --- | --- |
| `0x00` | AUTO OFF |
| `0x01` | AUTO ON |

현재 구현에서 `0x01` 요청은 `state.hvacMode = HVAC_AUTO`로 반영된다.

---

### Intake

| 값 | 설명 |
| --- | --- |
| `0x00` | Fresh Air |
| `0x01` | Recirculation |

현재 구현에서는 값 범위만 검증하며, 별도 상태 변수에는 아직 저장하지 않는다.

---

### Screen Mode

`Screen Mode`는 MKBD의 전환형 키보드 모드를 제어한다.

| 값 | 설명 | 코드 매핑 |
| --- | --- | --- |
| `0x00` | DATC 화면 | `SCREEN_DATC` |
| `0x01` | INFO 화면 | `SCREEN_INFO` |

HU는 이 Signal을 사용해 MKBD의 현재 화면과 버튼 키맵을 전환할 수 있다.

예:

```text
Signal = 0x08
Value  = 0x01
```

위 요청은 MKBD를 INFO 화면으로 전환한다.

---

### Media

`Media`는 INFO 화면의 Media 상태를 제어한다.

| 값 | 설명 |
| --- | --- |
| `0x00` | DEV / OFF |
| `0x01` | READY / ON |

HU는 이 Signal을 사용해 MKBD의 Media 상태를 변경할 수 있다.
MKBD에서 INFO 화면 D2 버튼으로 Media 상태가 바뀌면 같은 Signal로 Broadcast한다.

---

### Volume

`Volume`은 INFO 화면의 볼륨 값을 제어한다.

| 값 | 설명 |
| --- | --- |
| `0x00` | Volume 0 |
| `0x01` | Volume 1 |
| `...` | ... |
| `0x1E` | Volume 30 |

허용 범위:

```text
0x00 ~ 0x1E
```

HU는 이 Signal을 사용해 MKBD의 볼륨 값을 직접 설정할 수 있다.
MKBD에서 INFO 화면 D3/D4 버튼으로 볼륨이 바뀌면 같은 Signal로 Broadcast한다.

---

### Map

`Map`은 INFO 화면의 Map 상태를 제어한다.

| 값 | 설명 |
| --- | --- |
| `0x00` | DEV / OFF |
| `0x01` | READY / ON |

HU는 이 Signal을 사용해 MKBD의 Map 상태를 변경할 수 있다.
MKBD에서 INFO 화면 D5 버튼으로 Map 상태가 바뀌면 같은 Signal로 Broadcast한다.

---

## 12. Alive Counter

Alive Counter는 Byte6에 위치한다.

송신 측은 프레임을 보낼 때마다 Counter를 1씩 증가시킨다.

```text
0 -> 1 -> 2 -> 3 -> ...
```

현재 구현 정책:

- MKBD 송신 Counter는 `canTxCounter`로 관리한다.
- Counter 값은 1바이트이므로 `0xFF` 이후 자연스럽게 `0x00`으로 overflow될 수 있다.
- 수신 Counter 순서 검증은 아직 수행하지 않는다.
- Counter 순서 검증은 향후 확장 항목으로 둔다.

---

## 13. Checksum

Checksum은 Byte0부터 Byte6까지 XOR 연산하여 생성한다.

공식:

```cpp
checksum =
  byte0 ^
  byte1 ^
  byte2 ^
  byte3 ^
  byte4 ^
  byte5 ^
  byte6;
```

Byte7에는 계산된 Checksum 값을 넣는다.

수신 측은 동일한 방식으로 Checksum을 다시 계산하고 Byte7과 비교한다.

주의:

기존 초안의 일부 예시는 `0x26`처럼 Byte 합계에 가까운 값이 들어가 있었다.
본 명세서는 공식에 맞춰 XOR Checksum을 기준으로 예시 값을 정리한다.

---

## 14. 프레임 예시

### Fan Speed Write Request

Head Unit이 MKBD에 팬 속도를 3으로 설정하라고 요청하는 예시다.

CAN Frame:

| Field | 값 |
| --- | --- |
| CAN ID | `0x100` |
| DLC | `8` |
| Service | `0x20` |
| Result | `0x00` |
| Signal | `0x02` |
| Value | `0x03` |
| Option | `0x00` |
| Reserved | `0x00` |
| Counter | `0x01` |
| Checksum | `0x20` |

Raw Data:

```text
20 00 02 03 00 00 01 20
```

Checksum 계산:

```text
0x20 ^ 0x00 ^ 0x02 ^ 0x03 ^ 0x00 ^ 0x00 ^ 0x01 = 0x20
```

---

### Fan Speed Write Success Response

MKBD가 팬 속도 설정 요청을 정상 처리한 경우다.

CAN Frame:

| Field | 값 |
| --- | --- |
| CAN ID | `0x101` |
| DLC | `8` |
| Service | `0x22` |
| Result | `0x02` |
| Signal | `0x02` |
| Value | `0x03` |
| Option | `0x00` |
| Reserved | `0x00` |
| Counter | `0x01` |
| Checksum | `0x20` |

Raw Data:

```text
22 02 02 03 00 00 01 20
```

Checksum 계산:

```text
0x22 ^ 0x02 ^ 0x02 ^ 0x03 ^ 0x00 ^ 0x00 ^ 0x01 = 0x20
```

---

### Fan Speed Write Fail Response

팬 속도 값이 범위를 벗어난 경우다.

예:

```text
Fan Speed = 9
```

CAN Frame:

| Field | 값 |
| --- | --- |
| CAN ID | `0x101` |
| DLC | `8` |
| Service | `0x22` |
| Result | `0x0F` |
| Signal | `0x02` |
| Value | `0x09` |
| Option | `0x04` |
| Reserved | `0x00` |
| Counter | `0x01` |
| Checksum | `0x23` |

Raw Data:

```text
22 0F 02 09 04 00 01 23
```

Option 의미:

```text
0x04 = Value Out Of Range
```

---

### Checksum Error Response

Checksum이 틀린 요청을 수신한 경우다.

CAN Frame:

| Field | 값 |
| --- | --- |
| CAN ID | `0x101` |
| DLC | `8` |
| Service | `0x22` |
| Result | `0x0F` |
| Signal | 요청 프레임의 Signal |
| Value | 요청 프레임의 Value |
| Option | `0x01` |
| Reserved | `0x00` |
| Counter | 요청 프레임의 Counter |
| Checksum | Byte0~Byte6 XOR |

Option 의미:

```text
0x01 = Checksum Error
```

---

### Screen Mode Write Request

Head Unit이 MKBD를 INFO 화면으로 전환하는 예시다.

CAN Frame:

| Field | 값 |
| --- | --- |
| CAN ID | `0x100` |
| DLC | `8` |
| Service | `0x20` |
| Result | `0x00` |
| Signal | `0x08` |
| Value | `0x01` |
| Option | `0x00` |
| Reserved | `0x00` |
| Counter | `0x01` |
| Checksum | `0x28` |

Raw Data:

```text
20 00 08 01 00 00 01 28
```

Checksum 계산:

```text
0x20 ^ 0x00 ^ 0x08 ^ 0x01 ^ 0x00 ^ 0x00 ^ 0x01 = 0x28
```

---

### Volume Write Request

Head Unit이 MKBD의 INFO 화면 볼륨을 15로 설정하는 예시다.

CAN Frame:

| Field | 값 |
| --- | --- |
| CAN ID | `0x100` |
| DLC | `8` |
| Service | `0x20` |
| Result | `0x00` |
| Signal | `0x0A` |
| Value | `0x0F` |
| Option | `0x00` |
| Reserved | `0x00` |
| Counter | `0x01` |
| Checksum | `0x24` |

Raw Data:

```text
20 00 0A 0F 00 00 01 24
```

Checksum 계산:

```text
0x20 ^ 0x00 ^ 0x0A ^ 0x0F ^ 0x00 ^ 0x00 ^ 0x01 = 0x24
```

---

## 15. Broadcast

Broadcast CAN ID는 다음과 같다.

```text
0x300
```

Broadcast는 MKBD 상태를 HU 또는 다른 노드에 알리기 위한 선택 기능이다.

현재 구현은 전체 상태를 매번 보내지 않고, 실제 값이 바뀐 Signal만 Broadcast한다.

```text
Power
Fan Speed
Temperature
Mode
A/C
Auto
Screen Mode
Media
Volume
Map
```

예를 들어 Fan Speed만 바뀌면 `Fan Speed` 프레임만 송신한다.
Temperature만 바뀌면 `Temperature` 프레임만 송신한다.
INFO 화면의 Media, Volume, Map 값이 바뀌면 각각 `Media`, `Volume`, `Map` Signal로 Broadcast한다.

화면 전환은 `Screen Mode` Signal 값이 바뀌는 동작이므로 Broadcast한다.

HVAC 모드 변경은 여러 Signal을 바꿀 수 있다.
예를 들어 `OFF -> AC` 전환에서는 `Power`, `Fan Speed`, `A/C` 값이 함께 바뀔 수 있으므로 해당 Signal들만 Broadcast한다.

주의:

Broadcast는 필수 기능이 아니다.
HU가 필요할 때마다 Read Request를 보내는 방식으로도 상태 동기화가 가능하다.

포트폴리오 구현에서는 `0x100/0x101` 요청-응답 구조를 우선 검증하고, Broadcast는 확장 기능으로 설명할 수 있다.

---

## 16. 에러 처리 정책

### Payload 계층 에러

Payload 계층에서 처리하는 에러:

| 에러 | 처리 |
| --- | --- |
| Checksum Error | Fail Response 송신 |
| Unsupported Service | Fail Response 송신 |
| Unsupported Signal | Fail Response 송신 |
| Value Out Of Range | Fail Response 송신 |

Fail Response 형식:

```text
Byte1 Result = 0x0F
Byte4 Option = Error Code
```

---

### Frame 계층 에러

Frame 계층에서 처리하는 에러:

| 에러 | 처리 |
| --- | --- |
| Unknown CAN ID | Serial 로그 후 무시 |
| Invalid DLC | Serial 로그 후 무시 |
| Send Fail | Serial 로그 |
| MCP2515 Init Fail | Serial 로그 |

Invalid DLC에 응답하지 않는 이유:

```text
DLC가 8이 아니면 Signal, Value, Counter 필드를 신뢰할 수 없기 때문
```

---

## 17. 소프트웨어 아키텍처

CAN 통신 로직은 단위 테스트를 위해 하드웨어 의존 코드와 순수 Payload 처리 코드를 분리한다.

현재 프로젝트 구조:

| 파일 | 역할 | 단위 테스트 |
| --- | --- | --- |
| `GDS.h` | 공통 상수, 매직넘버 관리 | 가능 |
| `can/CanProtocol.cpp` | Payload 생성, 변환, Checksum | 가능 |
| `can/CanHandler.cpp` | Write/Read Request 처리, 상태 반영, 에러 응답 생성 | 가능 |
| `can/CanDriver.cpp` | MCP2515, `mcp_can`, SPI 송수신 래퍼 | 통합 테스트 |
| `state/State.cpp` | 공통 상태 초기화와 문자열 변환 | 가능 |
| `button/ButtonInput.cpp` | 버튼 edge/debounce 처리 | 가능 |
| `app/AppLogic.cpp` | 버튼 이벤트와 상태 변경 조립, PWM/LED 계산 | 가능 |
| `display/Datc.cpp` | DATC 화면 출력 | 통합 테스트 |
| `display/Info.cpp` | INFO 화면 출력 | 통합 테스트 |

설계 규칙:

- CAN Payload 처리 로직은 MCP2515에 직접 의존하지 않는다.
- `CanHandler`는 `mcp_can` 또는 SPI 함수를 호출하지 않는다.
- `CanProtocol`은 `SystemState`와 MCP2515를 모두 모른다.
- `CanDriver`는 Payload 의미를 해석하지 않는다.
- 단위 테스트는 `CanProtocol`, `CanHandler`, `AppLogic`, `ButtonInput` 같은 순수 로직 중심으로 작성한다.
- MCP2515 송수신은 실제 하드웨어를 연결한 통합 테스트에서 검증한다.

---

## 18. 향후 확장 항목

향후 확장 가능한 기능:

- DBC(Database CAN) 구조 정의
- Multi Node Architecture
- Diagnostic Frame
- UDS 스타일 진단 서비스
- Error Frame 처리
- Retry / Timeout State Machine
- Alive Counter 순서 검증
- CAN Bus Monitoring
- OTA Control via WiFi
