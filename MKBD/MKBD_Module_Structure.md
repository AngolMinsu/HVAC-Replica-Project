# MKBD 모듈 분리 구조

## 1. 분리 목적

MKBD 코드는 처음에는 `MKBD.ino` 중심으로 버튼 입력, 상태 변경, OLED 출력, 팬 제어, CAN 송수신이 함께 들어갈 수 있었다.

하지만 이렇게 구성하면 다음 문제가 생긴다.

- 버튼 로직을 따로 테스트하기 어렵다.
- CAN Payload 처리 로직을 하드웨어 없이 검증하기 어렵다.
- OLED 출력 코드와 상태 변경 코드가 섞인다.
- 기능이 늘어날수록 `MKBD.ino`가 너무 커진다.

그래서 현재 구조는 다음 원칙으로 분리했다.

```text
MKBD.ino는 조립만 담당한다.
상태 변경 로직은 별도 파일로 분리한다.
하드웨어 의존 코드와 순수 로직을 분리한다.
단위 테스트 가능한 함수는 최대한 하드웨어에 의존하지 않게 만든다.
```

---

## 2. 현재 폴더 구조

```text
MKBD/
  MKBD.ino
  GDS.h

  state/
    State.h
    State.cpp

  app/
    AppLogic.h
    AppLogic.cpp

  button/
    ButtonInput.h
    ButtonInput.cpp

  display/
    Datc.h
    Datc.cpp
    Info.h
    Info.cpp

  can/
    CanDriver.h
    CanDriver.cpp
    CanProtocol.h
    CanProtocol.cpp
    CanHandler.h
    CanHandler.cpp
    CanMonitor.h
    CanMonitor.cpp
```

---

## 3. 파일별 역할

### MKBD.ino

전체 기능을 조립하는 진입점이다.

주요 역할:

- Arduino `setup()` / `loop()`
- 핀 초기화
- OLED 초기화
- CAN 초기화
- 버튼 이벤트 수신
- 상태 변경 후 필요한 CAN Broadcast 호출
- LED / Fan PWM / OLED 갱신

`MKBD.ino`는 세부 로직을 직접 처리하지 않고, 각 모듈의 함수를 호출하는 역할에 집중한다.

---

### GDS.h

프로젝트 공통 상수를 모아둔 파일이다.

주요 내용:

- 핀 번호
- debounce/display 주기
- 팬 속도/온도/볼륨 범위
- CAN ID
- CAN Service / Result / Signal
- CAN Error Code

매직넘버를 줄이고, 명세 값이 바뀌었을 때 한 곳에서 관리하기 위해 사용한다.

---

### state/State

MKBD의 전체 상태를 관리한다.

주요 상태:

```cpp
screenMode
hvacMode
fanSpeed
setTemp
windMode
volume
mediaReady
mapReady
```

주요 함수:

- `initSystemState()`
- `toggleScreenMode()`
- `screenModeToText()`
- `hvacModeToText()`
- `windModeToText()`

---

### app/AppLogic

버튼 이벤트와 상태 변경을 연결하는 애플리케이션 로직이다.

주요 역할:

- 현재 화면 모드에 따라 DATC/INFO 버튼 핸들러 호출
- 팬 PWM 값 계산
- 상태 LED 값 계산
- 화면 갱신 주기 판단

예:

```cpp
handleButtonAction(state, button);
calculateFanPwm(state);
calculateStatusLed(state);
```

이 파일은 단위 테스트 대상으로 적합하다.

---

### button/ButtonInput

물리 버튼 입력을 이벤트로 바꾸는 모듈이다.

주요 역할:

- 이전 버튼 상태 저장
- 현재 버튼 상태와 비교
- `HIGH -> LOW` falling edge 감지
- debounce 시간 적용
- 어떤 버튼이 눌렸는지 반환

이 모듈은 실제 `digitalRead()`를 직접 호출하지 않는다.
`MKBD.ino`가 읽은 버튼 값을 전달받아 이벤트를 계산한다.

따라서 단위 테스트에서 가짜 버튼 입력을 넣어 검증할 수 있다.

---

### display/Datc

DATC 화면과 DATC 버튼 동작을 담당한다.

버튼 로직:

- D2: HVAC 모드 순환
- D3: Fan Speed 순환
- D4: Temperature 순환
- D5: Wind Mode 순환

화면 출력:

- HVAC 모드
- 설정 온도
- 팬 속도
- 풍향 모드
- 팬 바 표시

주의:

```text
datcHandleButton* 함수는 단위 테스트 가능
datcDrawScreen 함수는 OLED 의존성이 있으므로 통합 테스트 대상
```

---

### display/Info

INFO 화면과 INFO 버튼 동작을 담당한다.

버튼 로직:

- D2: Media 상태 토글
- D3: Volume 증가
- D4: Volume 감소
- D5: Map 상태 토글

화면 출력:

- Media 상태
- Volume 값
- Volume Bar
- Map 상태

주의:

```text
infoHandleButton* 함수는 단위 테스트 가능
infoDrawScreen 함수는 OLED 의존성이 있으므로 통합 테스트 대상
```

---

### can/CanDriver

MCP2515 하드웨어 송수신을 담당하는 계층이다.

내부적으로 `mcp_can` 라이브러리를 사용한다.

주요 역할:

- MCP2515 초기화
- CAN 프레임 송신
- CAN 프레임 수신

중요한 점:

```text
CanDriver는 Payload 의미를 해석하지 않는다.
```

이 계층은 실제 MCP2515와 SPI에 의존하므로 통합 테스트 대상이다.

---

### can/CanProtocol

CAN Payload 규칙을 담당한다.

주요 역할:

- Payload 구조 정의
- CAN ID / Service / Result / Signal / Error Code 정의
- Payload 생성
- byte 배열 변환
- XOR Checksum 계산
- Checksum 검증

중요한 점:

```text
CanProtocol은 MCP2515를 모른다.
CanProtocol은 SystemState를 모른다.
```

따라서 단위 테스트 대상으로 적합하다.

---

### can/CanHandler

수신된 CAN Payload를 해석해서 `SystemState`에 반영한다.

주요 역할:

- Write Request 처리
- Read Request 처리
- Checksum 검증
- Signal/Value 범위 검증
- 성공/실패 응답 생성
- Error Code 설정

중요한 점:

```text
CanHandler는 mcp_can 또는 SPI를 호출하지 않는다.
```

따라서 Head Unit 없이도 Payload 단위 테스트가 가능하다.

---

### can/CanMonitor

Serial Monitor에 CAN RX/TX 프레임을 출력하는 디버깅 전용 모듈이다.

예시 출력:

```text
CAN RX ID:0x100 DLC:8 DATA: 20 00 02 03 00 00 01 20
CAN TX ID:0x101 DLC:8 DATA: 22 02 02 03 00 00 01 20
CAN TX ID:0x300 DLC:8 DATA: 32 00 0A 0F 00 00 03 34
```

이 모듈은 테스트 대상이 아니라 개발 중 모니터링을 위한 보조 모듈이다.

---

## 4. 데이터 흐름

### 버튼 입력 흐름

```text
digitalRead()
  -> ButtonInput
  -> AppLogic
  -> Datc 또는 Info 핸들러
  -> SystemState 변경
  -> 변경된 Signal만 CAN Broadcast
  -> OLED / LED / Fan 갱신
```

---

### CAN 수신 흐름

```text
MCP2515
  -> CanDriver
  -> CanProtocol
  -> CanHandler
  -> SystemState 변경
  -> Control Response 송신
  -> 변경된 Signal만 Broadcast
```

---

### CAN 송신 흐름

```text
SystemState 변경
  -> CanHandler 또는 MKBD.ino
  -> CanProtocol로 Payload 생성
  -> CanDriver로 CAN Frame 송신
  -> CanMonitor로 Serial 출력
```

---

## 5. 단위 테스트 대상

단위 테스트가 적합한 모듈:

```text
State
AppLogic
ButtonInput
Datc button handlers
Info button handlers
CanProtocol
CanHandler
```

통합 테스트가 필요한 모듈:

```text
CanDriver
CanMonitor
OLED draw functions
실제 MCP2515 CAN 송수신
```

---

## 6. 면접 설명 요약

이 프로젝트는 `MKBD.ino`에 모든 코드를 넣지 않고, 입력 처리, 상태 변경, 화면 출력, CAN 송수신, Payload 해석을 모듈별로 분리했다.

특히 MCP2515 하드웨어 송수신 계층과 CAN Payload 처리 계층을 분리해서, 실제 CAN 하드웨어 없이도 핵심 프로토콜 로직을 단위 테스트할 수 있게 설계했다.

