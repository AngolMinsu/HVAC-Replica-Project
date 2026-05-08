# Web UI 제작기 및 코드 설명

## 1. 제작 목적

처음 HU는 Serial Monitor에 명령어를 입력해서 CAN 프레임을 보내는 테스트 노드였다.

예시:

```text
screen info
fan 3
volume 15
```

이 방식은 기능 검증에는 충분하지만, Head Unit처럼 보이거나 사용되지는 않는다. 그래서 ESP8266이 직접 웹 서버를 열고, PC나 스마트폰 브라우저에서 HVAC/INFO 기능을 제어할 수 있도록 Web UI를 추가했다.

목표는 다음과 같다.

- 별도 모바일 앱 없이 브라우저로 제어
- ESP8266 단독으로 접속 가능한 AP 제공
- 웹 버튼/슬라이더 입력을 CAN Request로 변환
- MKBD의 Response/Broadcast를 웹 상태에 반영
- 포트폴리오 시연 시 Head Unit 역할을 직관적으로 보여주기

## 2. 전체 구조

```text
Browser
  -> ESP8266 Web UI
  -> Web API
  -> CAN Payload 생성
  -> MCP2515 CAN 송신
  -> MKBD
```

수신 흐름은 반대 방향이다.

```text
MKBD
  -> CAN Response / Broadcast
  -> ESP8266 CAN 수신
  -> Payload 해석
  -> Web UI 상태 캐시 갱신
  -> Browser polling으로 화면 반영
```

## 3. 관련 파일

| 파일 | 역할 |
| --- | --- |
| `HU.ino` | setup/loop, CAN 송수신 조립, Web UI 호출 |
| `WebUi.h` | Web UI 모듈 외부 인터페이스 |
| `WebUi.cpp` | Wi-Fi, Web Server, HTML, API, 상태 캐시 |
| `CanProtocol.*` | CAN Payload 생성/변환/Checksum |
| `CanDriver.*` | MCP2515 송수신 |
| `GDS.h` | Wi-Fi, CAN, 범위 상수 |

## 4. Wi-Fi 동작 방식

HU는 AP+STA 모드로 동작한다.

AP 모드:

```text
SSID: HVAC-HU
Password: 12345678
URL: http://192.168.4.1
```

이 AP는 항상 켜져 있다. 외부 Wi-Fi 연결에 실패하더라도 사용자는 `HVAC-HU`에 접속해서 다시 설정할 수 있다.

STA 모드:

```text
사용자가 웹 UI에서 주변 Wi-Fi를 선택
비밀번호 입력
ESP8266이 해당 Wi-Fi에 접속
연결된 SSID / IP / RSSI를 웹 UI에 표시
```

입력한 Wi-Fi 정보는 EEPROM에 저장한다. 재부팅 후에는 저장된 Wi-Fi로 자동 접속을 시도한다.

## 5. Web UI 화면 구성

웹 UI는 하나의 HTML 문자열로 `WebUi.cpp` 안에 포함되어 있다.

주요 섹션:

- Status
- Wi-Fi
- Screen
- HVAC
- INFO

Status 영역은 현재 HU가 알고 있는 MKBD 상태를 보여준다.

```text
Screen
Power
Fan
Temp
Wind
Volume
```

Wi-Fi 영역은 현재 연결 상태와 네트워크 변경 기능을 제공한다.

```text
AP SSID
Connected SSID
Station IP
RSSI
Scan
Connect
Disconnect
```

HVAC/INFO 영역은 실제 Head Unit 버튼 역할을 한다.

## 6. Web API

웹 UI는 내부적으로 ESP8266의 API를 호출한다.

### 상태 조회

```text
GET /api/state
```

HU 내부 상태 캐시를 JSON으로 반환한다.

예시:

```json
{
  "canReady": 1,
  "power": 1,
  "fan": 3,
  "temp": 24,
  "wind": 0,
  "ac": 1,
  "auto": 0,
  "screen": 1,
  "media": 1,
  "volume": 15,
  "map": 0
}
```

### CAN Write Request

```text
GET /api/write?signal={signal}&value={value}
```

웹 버튼 또는 슬라이더 조작 시 호출된다.

예시:

```text
GET /api/write?signal=8&value=1
```

의미:

```text
Screen Mode = INFO
```

이 요청은 내부에서 다음 흐름으로 이어진다.

```text
handleWrite()
  -> requestSender(CAN_SERVICE_WRITE_REQUEST, signal, value)
  -> sendRequest()
  -> canMakePayload()
  -> canDriverSend()
```

### CAN Read Request

```text
GET /api/read?signal={signal}
```

특정 Signal의 현재 값을 MKBD에 요청한다. 현재 웹 UI는 주로 Response/Broadcast 수신과 polling으로 상태를 갱신하므로, Read API는 디버깅과 확장용 성격이 강하다.

### Wi-Fi 상태 조회

```text
GET /api/wifi
```

현재 AP 정보와 외부 Wi-Fi 연결 상태를 반환한다.

### Wi-Fi 검색

```text
GET /api/wifi/scan
```

주변 Wi-Fi 목록을 검색한다.

### Wi-Fi 연결

```text
POST /api/wifi/connect
```

Form field:

```text
ssid
password
```

### Wi-Fi 연결 해제

```text
POST /api/wifi/disconnect
```

외부 Wi-Fi 연결을 해제하고 EEPROM에 저장된 Wi-Fi 정보를 삭제한다.

## 7. 코드 핵심 설명

### 7.1 Web UI 시작

`HU.ino`의 `setup()`에서 Web UI를 시작한다.

```cpp
webUiBegin(sendRequest);
```

`sendRequest` 함수 포인터를 Web UI 모듈에 넘긴다. 이렇게 하면 `WebUi.cpp`는 CAN Driver를 직접 조립하지 않고, HU의 기존 송신 흐름을 재사용할 수 있다.

### 7.2 Web Server 처리

`HU.ino`의 `loop()`에서는 매 반복마다 웹 요청을 처리한다.

```cpp
webUiHandle();
processSerialCommand();
processCanReceive();
```

웹 UI는 별도 스레드가 아니라 loop 안에서 계속 `server.handleClient()`를 호출하는 방식으로 동작한다.

### 7.3 CAN 수신 상태 반영

MKBD에서 Response 또는 Broadcast가 오면 `processCanReceive()`에서 Payload를 해석한다.

```cpp
CanPayload payload = canPayloadFromBytes(frame.data);
webUiUpdateFromPayload(payload);
```

`webUiUpdateFromPayload()`는 Checksum과 Result를 확인한 뒤, Signal에 맞는 상태 값을 갱신한다.

예시:

```text
Signal = Volume
Value = 15
  -> webState.volume = 15
```

### 7.4 상태 캐시

웹 UI는 `HuWebState` 구조체로 현재 상태를 보관한다.

```cpp
struct HuWebState {
  uint8_t power;
  uint8_t fan;
  uint8_t temp;
  uint8_t wind;
  uint8_t ac;
  uint8_t autoMode;
  uint8_t screen;
  uint8_t media;
  uint8_t volume;
  uint8_t map;
};
```

브라우저는 `/api/state`를 주기적으로 호출해서 이 값을 화면에 반영한다.

```text
Polling interval: 1000 ms
```

## 8. Wi-Fi 설정 코드 설명

Wi-Fi 정보는 EEPROM에 고정 길이 문자열로 저장한다.

```text
0~31     SSID
32~95    Password
```

재부팅 시 저장된 SSID가 있으면 자동으로 연결을 시도한다.

```text
loadWifiCredentials()
  -> beginStationConnect()
  -> WiFi.begin()
```

연결이 성공하면 Serial Monitor에 출력된다.

```text
WIFI STA CONNECTED:...
WIFI STA IP:...
```

15초 안에 연결하지 못하면 timeout 로그를 출력한다.

```text
WIFI STA CONNECT TIMEOUT
```

이때도 AP 모드는 계속 유지되므로 `192.168.4.1` 접속은 가능하다.

## 9. 설계상 선택

### AP를 항상 유지한 이유

외부 Wi-Fi 설정이 틀리면 일반적인 Wi-Fi Client 장치는 접속 방법을 잃어버릴 수 있다. HU는 AP를 항상 유지해서, 사용자가 언제든지 `HVAC-HU`로 접속해 설정을 복구할 수 있게 했다.

### WebSocket을 쓰지 않은 이유

현재 상태 갱신 주기는 빠른 실시간성이 필요하지 않다. CAN 상태는 사람이 조작하는 버튼 단위로 바뀌므로, 1초 polling만으로도 시연과 테스트에는 충분하다.

WebSocket을 쓰면 구조는 더 실시간에 가까워지지만 코드 복잡도가 증가한다. 현재 버전은 안정성과 설명 가능성을 우선했다.

### 별도 앱을 만들지 않은 이유

브라우저 기반 UI는 별도 앱 설치가 필요 없다. PC, 스마트폰, 태블릿에서 같은 방식으로 접속할 수 있고, 포트폴리오 시연에서도 접근성이 좋다.

## 10. 현재 제한 사항

- 웹 UI는 ESP8266 내부 HTML 문자열로 제공된다.
- CSS/JS 파일을 별도로 분리하지 않았다.
- 상태 갱신은 WebSocket이 아니라 polling 방식이다.
- Wi-Fi 비밀번호는 EEPROM에 평문으로 저장된다.
- 보안 인증은 없다.
- 복잡한 차량 UI가 아니라 CAN 기능 검증용 Head Unit UI이다.

## 11. 향후 개선 후보

- 최근 CAN 로그를 웹 UI에 표시
- WebSocket 기반 실시간 상태 갱신
- HTML/CSS/JS를 LittleFS로 분리
- 버튼 UI를 실제 Head Unit 패널처럼 개선
- Wi-Fi 설정값 초기화 버튼 추가

## 12. 면접 설명 요약

HU Web UI는 ESP8266이 직접 제공하는 브라우저 기반 Head Unit 인터페이스이다.

사용자는 별도 앱 없이 `HVAC-HU` Wi-Fi에 접속해 `192.168.4.1`에서 HVAC/INFO 기능을 제어할 수 있다. 웹 UI 입력은 ESP8266 내부 API를 거쳐 CAN Payload로 변환되고, MCP2515를 통해 MKBD로 송신된다.

MKBD의 Response와 Broadcast는 HU의 상태 캐시에 반영되고, 브라우저는 `/api/state`를 polling해 화면을 갱신한다. AP를 항상 유지하면서 외부 Wi-Fi 연결도 지원하도록 설계해, 시연 환경에서 접속 안정성과 복구 가능성을 확보했다.
