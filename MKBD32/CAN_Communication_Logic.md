# CAN 통신 로직 설명서

## 1. 전체 개념

이 프로젝트의 CAN 통신은 크게 두 계층으로 분리한다.

```text
하드웨어 / 드라이버 계층
  - MCP2515 모듈
  - SPI 통신
  - mcp_can 라이브러리
  - Raw CAN Frame 송수신

Payload / Protocol 계층
  - 8바이트 Payload 의미 해석
  - Service / Result / Signal / Value 처리
  - Checksum 검증
  - SystemState 변경
  - 응답 Payload 생성
```

비유하면 다음과 같다.

```text
mcp_can 라이브러리 = 운전기사
프로젝트 CAN 프로토콜 = 대화 내용
```

MCP2515 드라이버는 CAN 프레임을 받아오고 보내는 역할만 한다.
Byte0이나 Byte2가 어떤 의미인지는 판단하지 않는다.

Payload 처리 로직은 수신된 8바이트의 의미만 해석한다.
MCP2515 함수나 SPI 함수를 직접 호출하지 않는다.

---

## 2. 파일 역할

```text
GDS.h
can/
  CanDriver.h / CanDriver.cpp
  CanProtocol.h / CanProtocol.cpp
  CanHandler.h / CanHandler.cpp
```

### GDS.h

`GDS.h`는 프로젝트 공통 상수를 저장하는 파일이다.

주요 역할:

- 하드웨어 핀 번호
- 시간 설정값
- HVAC / INFO 값 범위
- CAN ID
- CAN Service / Result / Signal 값
- CAN Error Code 값

이 파일을 두면 로직 파일 내부에 매직넘버가 흩어지는 것을 줄일 수 있다.

---

### CanDriver

`CanDriver`는 하드웨어와 직접 맞닿는 계층이다.

설치된 `mcp_can` 라이브러리를 감싸서 사용한다.

주요 역할:

- MCP2515 초기화
- CAN 프레임 수신 여부 확인
- MCP2515에서 CAN 프레임 읽기
- MCP2515를 통해 CAN 프레임 송신

중요한 점:

```text
CanDriver는 Payload 의미를 해석하지 않는다.
```

예시:

```cpp
CanFrame rxFrame;
canDriverReceive(rxFrame);
```

내부적으로는 다음과 같은 `mcp_can` 함수를 사용한다.

```cpp
mcp_can.checkReceive();
mcp_can.readMsgBuf(...);
mcp_can.sendMsgBuf(...);
```

---

### CanProtocol

`CanProtocol`은 프로젝트 전용 CAN 메시지 규칙을 정의한다.

이 계층은 8바이트 Payload 구조를 알고 있다.

| Byte | 필드 |
| --- | --- |
| Byte0 | Service |
| Byte1 | Result |
| Byte2 | Signal |
| Byte3 | Value |
| Byte4 | Option |
| Byte5 | Reserved |
| Byte6 | Counter |
| Byte7 | Checksum |

주요 역할:

- CAN ID 정의
- Service 값 정의
- Result 값 정의
- Signal 값 정의
- Error Code 값 정의
- Payload 생성
- Payload 구조체를 byte 배열로 변환
- byte 배열을 Payload 구조체로 변환
- XOR Checksum 계산
- Checksum 검증

예시:

```cpp
CanPayload payload =
  canMakePayload(
    CAN_SERVICE_WRITE_RESPONSE,
    CAN_RESULT_SUCCESS,
    CAN_SIGNAL_FAN_SPEED,
    3,
    0,
    counter
  );
```

중요한 점:

```text
CanProtocol은 SystemState를 모른다.
CanProtocol은 MCP2515를 모른다.
```

즉, 이 파일은 byte 수준의 메시지 규칙만 담당한다.

---

### CanHandler

`CanHandler`는 CAN Payload와 애플리케이션 상태를 연결한다.

주요 역할:

- 수신 요청의 Checksum 검증
- Write Request 처리
- Read Request 처리
- `SystemState` 변경
- 응답 Payload 생성
- HVAC 상태 Broadcast Payload 생성

예시:

```cpp
CanPayload request = canPayloadFromBytes(rxFrame.data);
CanPayload response;

uint8_t changed =
  canProcessControlRequest(state, request, response);
```

중요한 점:

```text
CanHandler는 MCP2515를 호출하지 않는다.
CanHandler는 SPI를 호출하지 않는다.
```

따라서 `CanHandler`는 단위 테스트 대상으로 적합하다.

---

## 3. 실행 흐름

### Head Unit에서 제어 요청을 받는 경우

```text
HU
  -> CAN ID 0x100
  -> MKBD
```

실행 흐름:

```text
1. CanDriver가 MCP2515에서 Raw CAN Frame을 읽는다.
2. MKBD.ino가 CAN ID와 DLC를 확인한다.
3. CanProtocol이 8바이트 데이터를 CanPayload로 변환한다.
4. CanHandler가 요청을 검증하고 SystemState에 반영한다.
5. CanProtocol이 응답 Payload를 생성한다.
6. CanDriver가 MCP2515를 통해 응답 Frame을 송신한다.
```

코드 흐름:

```cpp
CanFrame rxFrame;
canDriverReceive(rxFrame);

CanPayload request = canPayloadFromBytes(rxFrame.data);
CanPayload response;

uint8_t changed =
  canProcessControlRequest(state, request, response);

sendCanPayload(CAN_ID_CONTROL_RESPONSE, response);
```

---

## 4. CAN ID

| CAN ID | 방향 | 의미 |
| --- | --- | --- |
| `0x100` | HU -> MKBD | Control Request |
| `0x101` | MKBD -> HU | Control Response |
| `0x300` | MKBD -> ALL | HVAC Status Broadcast |

원본 명세서의 CAN ID 표와 예시에는 방향이 약간 다르게 보이는 부분이 있다.

현재 구현은 Fan Speed Write Request 예시 흐름을 따른다.

```text
0x100 = Head Unit이 MKBD에 제어 요청
0x101 = MKBD가 Head Unit에 제어 응답
0x300 = MKBD가 HVAC 상태 Broadcast
```

---

## 5. Payload 형식

이 프로젝트의 CAN Payload는 항상 8바이트 고정 길이를 사용한다.

```text
Byte0 Service
Byte1 Result
Byte2 Signal
Byte3 Value
Byte4 Option
Byte5 Reserved
Byte6 Counter
Byte7 Checksum
```

### Checksum

Checksum은 Byte0부터 Byte6까지 XOR 연산해서 만든다.

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

수신된 Payload는 다음 조건을 만족해야 정상으로 본다.

```cpp
canCalculateChecksum(payload) == payload.checksum
```

---

## 6. Service 값

| 값 | 이름 |
| --- | --- |
| `0x20` | Write Request |
| `0x22` | Write Response |
| `0x30` | Read Request |
| `0x32` | Read Response |

---

## 7. Result 값

| 값 | 이름 |
| --- | --- |
| `0x00` | Normal |
| `0x02` | Success |
| `0x0F` | Fail |

---

## 8. Error Code 값

`Result = 0x0F`인 경우 Byte4 `Option`에는 실패 원인을 나타내는 Error Code가 들어간다.

| 값 | 이름 |
| --- | --- |
| `0x00` | No Error |
| `0x01` | Checksum Error |
| `0x02` | Unsupported Service |
| `0x03` | Unsupported Signal |
| `0x04` | Value Out Of Range |
| `0x05` | Invalid DLC |
| `0x06` | Counter Error |
| `0x07` | Driver Error |

`Invalid DLC`는 Payload 파싱 전에 `MKBD.ino`에서 처리한다.
현재 구현에서는 Serial 로그만 남기고 응답은 보내지 않는다.

이유는 DLC가 8이 아니면 Signal, Value, Counter 필드 자체를 신뢰하기 어렵기 때문이다.

송신 실패 같은 Driver Error도 상대 노드에 Payload로 알리기 어렵다.
따라서 현재 구현에서는 로컬 Serial 진단 로그로 처리한다.

---

## 9. Signal 값

| 값 | Signal | 현재 매핑 |
| --- | --- | --- |
| `0x01` | Power | HVAC OFF / ON |
| `0x02` | Fan Speed | `state.fanSpeed` |
| `0x03` | Temperature | `state.setTemp` |
| `0x04` | Mode | `state.windMode` |
| `0x05` | A/C | `state.hvacMode == HVAC_AC` |
| `0x06` | Auto | `state.hvacMode == HVAC_AUTO` |
| `0x07` | Intake | 현재는 0 또는 1만 허용하는 Placeholder |
| `0x08` | Screen Mode | `state.screenMode` |
| `0x09` | Media | `state.mediaReady` |
| `0x0A` | Volume | `state.volume` |
| `0x0B` | Map | `state.mapReady` |

---

## 10. Write Request 처리

Write Request는 다음 Service 값을 사용한다.

```text
Service = 0x20
```

예시 요청:

```text
20 00 02 03 00 00 01 20
```

의미:

```text
Service  = Write Request
Signal   = Fan Speed
Value    = 3
Counter  = 1
Checksum = 0x20
```

이 요청은 다음 상태 변경을 의미한다.

```cpp
state.fanSpeed = 3;
```

이후 MKBD는 Write Response를 생성한다.

```text
Service = 0x22
Result  = 0x02 또는 0x0F
Signal  = 요청받은 Signal
Value   = 요청받은 Value
Option  = 성공이면 0x00, 실패이면 Error Code
```

예를 들어 Fan Speed 값으로 `9`가 들어오면 범위 오류이므로 다음 의미의 응답을 만든다.

```text
Result = 0x0F
Option = 0x04  // Value Out Of Range
```

---

## 11. Read Request 처리

Read Request는 다음 Service 값을 사용한다.

```text
Service = 0x30
```

Handler는 요청받은 Signal에 해당하는 값을 `SystemState`에서 읽는다.

예시:

```text
Signal = CAN_SIGNAL_TEMPERATURE
```

반환 값:

```cpp
state.setTemp
```

응답 Service는 다음 값을 사용한다.

```text
Service = 0x32
```

---

## 12. Broadcast 처리

MKBD 상태가 버튼 또는 CAN 요청으로 변경되면 HVAC 상태를 Broadcast할 수 있다.

Broadcast CAN ID:

```text
0x300
```

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
온도만 바뀌면 `Temperature` 프레임만 송신한다.
INFO 화면의 Media, Volume, Map 값이 바뀌면 각각 `Media`, `Volume`, `Map` Signal로 Broadcast한다.

화면 전환은 `Screen Mode` Signal 값이 바뀌는 동작이므로 Broadcast한다.

HVAC 모드 변경은 여러 Signal을 바꿀 수 있다.
예를 들어 `OFF -> AC` 전환에서는 `Power`, `Fan Speed`, `A/C` 값이 함께 바뀔 수 있으므로 해당 Signal들만 Broadcast한다.

HU도 같은 Signal들을 사용해 INFO 기능을 제어할 수 있다.

```text
Media  = 0x09, Value 0 또는 1
Volume = 0x0A, Value 0~30
Map    = 0x0B, Value 0 또는 1
```

각 Broadcast Frame은 `canTxCounter`를 1씩 증가시킨다.

Broadcast는 필수 기능은 아니다.
HU가 필요할 때마다 Read Request를 보내는 방식으로도 동기화할 수 있다.

---

## 13. 단위 테스트 대상

단위 테스트는 Payload 계층에 집중한다.

권장 테스트 대상:

```text
CanProtocol
  - canCalculateChecksum()
  - canValidateChecksum()
  - canMakePayload()
  - canPayloadToBytes()
  - canPayloadFromBytes()

CanHandler
  - canValidateWriteRequest()
  - canApplyWriteRequest()
  - canValidateReadRequest()
  - canApplyReadRequest()
  - canProcessControlRequest()
  - canMakeStatusPayload()
```

MCP2515 하드웨어 통신은 이 계층에서 단위 테스트하지 않는다.
그 부분은 실제 보드를 연결한 통합 테스트 영역이다.

예시 테스트:

```cpp
SystemState state;
initSystemState(state);

CanPayload request =
  canMakePayload(
    CAN_SERVICE_WRITE_REQUEST,
    CAN_RESULT_NORMAL,
    CAN_SIGNAL_FAN_SPEED,
    3,
    0,
    1
  );

CanPayload response;
uint8_t changed =
  canProcessControlRequest(state, request, response);

// 기대 결과:
// changed == 1
// state.fanSpeed == 3
// response.result == CAN_RESULT_SUCCESS
// response.option == CAN_ERROR_NONE
```

에러 테스트 예:

```text
Checksum 오류
  -> Result = CAN_RESULT_FAIL
  -> Option = CAN_ERROR_CHECKSUM

알 수 없는 Service
  -> Result = CAN_RESULT_FAIL
  -> Option = CAN_ERROR_UNSUPPORTED_SERVICE

알 수 없는 Signal
  -> Result = CAN_RESULT_FAIL
  -> Option = CAN_ERROR_UNSUPPORTED_SIGNAL

Fan Speed 9
  -> Result = CAN_RESULT_FAIL
  -> Option = CAN_ERROR_VALUE_OUT_OF_RANGE

Temperature 31
  -> Result = CAN_RESULT_FAIL
  -> Option = CAN_ERROR_VALUE_OUT_OF_RANGE
```

---

## 14. 의존성 방향

올바른 의존성 방향:

```text
MKBD.ino
  -> CanDriver
  -> CanProtocol
  -> CanHandler

CanDriver
  -> mcp_can 라이브러리
  -> SPI
  -> MCP2515 하드웨어

CanHandler
  -> CanProtocol
  -> SystemState

CanProtocol
  -> byte 규칙만 담당
```

피해야 할 의존성 방향:

```text
CanHandler -> mcp_can
CanProtocol -> MCP2515
Payload parser -> SPI
```

현재 구조는 이 잘못된 의존성 방향을 피하도록 설계되어 있다.
