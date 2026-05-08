# HU Specification

## 1. 목적

HU는 ESP8266을 이용해 Head Unit 역할을 수행하는 웹 기반 CAN 제어 노드이다.

MKBD 유닛은 버튼 입력, OLED 표시, HVAC/INFO 상태 관리를 담당한다. HU 유닛은 웹 UI에서 받은 사용자 입력을 CAN 제어 요청으로 변환해 MKBD로 송신하고, MKBD의 응답과 상태 Broadcast를 수신해 웹 UI 상태에 반영한다.

```text
Browser
  -> ESP8266 HU Web UI
  -> MCP2515 CAN
  -> MKBD
```

## 2. 시스템 구성

| 구성 | 역할 |
| --- | --- |
| Browser | 사용자가 HVAC/INFO 상태를 제어하는 UI |
| ESP8266 HU | Web Server, Web API, CAN Request 생성 |
| MCP2515 | CAN Frame 송수신 |
| MKBD | CAN Request 처리, 상태 변경, Response/Broadcast 송신 |

HU는 MKBD의 상태를 직접 결정하는 주체가 아니다. HU는 요청을 보내고, 최종 상태는 MKBD가 처리한 응답 또는 Broadcast를 기준으로 동기화한다.

## 3. Wi-Fi 동작 방식

현재 HU는 AP+STA 모드로 동작한다.

| 항목 | 값 |
| --- | --- |
| SSID | `HVAC-HU` |
| Password | `12345678` |
| Web URL | `http://192.168.4.1` |
| Port | `80` |

AP 모드를 사용하면 공유기 없이 PC 또는 스마트폰이 ESP8266에 직접 접속할 수 있다. 포트폴리오 시연 환경에서 네트워크 의존성을 줄이기 위한 선택이다.

STA 모드는 사용자가 웹 UI에서 선택한 외부 Wi-Fi에 HU를 연결하기 위해 사용한다. AP는 항상 유지되므로 외부 Wi-Fi 연결이 실패해도 사용자는 `http://192.168.4.1`로 다시 접속할 수 있다.

저장된 Wi-Fi 정보는 EEPROM에 보관하며, 재부팅 후 자동 접속을 시도한다.

## 4. Web UI 기능

웹 UI는 다음 제어 기능을 제공한다.

| 기능 | CAN Signal |
| --- | --- |
| DATC / INFO 화면 전환 | `Screen Mode` |
| Power On / Off | `Power` |
| Fan Speed `0~8` | `Fan Speed` |
| Temperature `18~30` | `Temperature` |
| Wind Mode `FACE / FOOT / DEF / MIX` | `Mode` |
| A/C On / Off | `A/C` |
| Auto On / Off | `Auto` |
| Media Ready / Dev | `Media` |
| Volume `0~30` | `Volume` |
| Map Ready / Dev | `Map` |

## 5. Web API

### 5.1 상태 조회

```text
GET /api/state
```

HU가 현재 알고 있는 상태를 JSON으로 반환한다.

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

### 5.2 상태 변경 요청

```text
GET /api/write?signal={signal}&value={value}
```

Web UI에서 사용자가 버튼 또는 슬라이더를 조작하면 HU는 이 API를 통해 CAN Write Request를 생성한다.

예시:

```text
GET /api/write?signal=8&value=1
```

의미:

```text
Screen Mode = INFO
```

### 5.3 상태 읽기 요청

```text
GET /api/read?signal={signal}
```

특정 Signal에 대한 CAN Read Request를 MKBD로 송신한다.

현재 웹 UI는 주로 Broadcast와 Response를 통해 상태를 갱신하므로, Read API는 디버깅 또는 확장용으로 둔다.

### 5.4 Wi-Fi 상태 조회

```text
GET /api/wifi
```

현재 AP 정보와 STA 연결 상태를 반환한다.

### 5.5 Wi-Fi 검색

```text
GET /api/wifi/scan
```

주변 Wi-Fi 목록을 검색한다.

### 5.6 Wi-Fi 연결

```text
POST /api/wifi/connect
```

Form field:

```text
ssid
password
```

입력한 Wi-Fi 정보는 EEPROM에 저장된다.

### 5.7 Wi-Fi 연결 해제

```text
POST /api/wifi/disconnect
```

STA 연결을 해제하고 저장된 Wi-Fi 정보를 삭제한다. AP 모드는 계속 유지된다.

## 6. CAN 통신 책임

HU는 공통 CAN Payload 명세를 그대로 사용한다. Payload 자체의 의미와 값 범위는 MKBD의 `CanSpec.md`를 기준으로 한다.

| CAN ID | 방향 | 의미 |
| --- | --- | --- |
| `0x100` | HU -> MKBD | Control Request |
| `0x101` | MKBD -> HU | Control Response |
| `0x300` | MKBD -> HU | Status Broadcast |

HU는 다음 책임만 가진다.

- Web UI 입력을 CAN Request로 변환
- CAN Frame 송신
- MKBD Response 수신
- MKBD Broadcast 수신
- 수신 Payload를 웹 상태 캐시에 반영

HU는 다음 책임을 가지지 않는다.

- MKBD 내부 상태의 최종 판정
- HVAC 동작 규칙 결정
- CAN Payload 규격 자체의 소유
- MCP2515 없는 Payload 단위 테스트 대상

## 7. Payload 구조

HU는 8바이트 고정 Payload를 사용한다.

| Byte | 의미 |
| --- | --- |
| Byte0 | Service |
| Byte1 | Result |
| Byte2 | Signal |
| Byte3 | Value |
| Byte4 | Option |
| Byte5 | Reserved |
| Byte6 | Counter |
| Byte7 | Checksum |

Checksum은 Byte0부터 Byte6까지 XOR한 값이다.

HU가 송신하는 주요 Service:

| Service | 의미 |
| --- | --- |
| `0x20` | Write Request |
| `0x30` | Read Request |

HU가 수신하는 주요 Service:

| Service | 의미 |
| --- | --- |
| `0x22` | Write Response |
| `0x32` | Read Response 또는 Status Broadcast |

## 8. 상태 동기화 방식

웹 UI의 상태는 HU 내부의 상태 캐시를 기준으로 표시된다.

상태 캐시는 다음 경우에 갱신된다.

1. 사용자가 웹 UI에서 제어 요청을 보냈을 때
2. MKBD에서 Control Response를 수신했을 때
3. MKBD에서 Status Broadcast를 수신했을 때

기본 흐름:

```text
Web UI 조작
  -> /api/write
  -> HU CAN Write Request 송신
  -> MKBD 상태 변경
  -> MKBD Response 또는 Broadcast 송신
  -> HU 상태 캐시 갱신
  -> Web UI polling으로 화면 반영
```

현재 웹 UI는 `/api/state`를 주기적으로 polling한다.

```text
Polling interval: 1000 ms
```

## 9. Serial Monitor 역할

Serial Monitor는 제어 UI가 아니라 진단용 로그 채널이다.

출력 내용:

- CAN 초기화 결과
- Web AP 접속 정보
- CAN TX Frame
- CAN RX Frame
- Payload 요약
- Checksum 결과

Serial 명령어도 유지하지만, 현재 HU의 주 인터페이스는 웹 UI이다.

## 10. 제한 사항

현재 HU는 포트폴리오 시연과 MKBD 통신 검증을 위한 버전이다.

제한 사항:

- AP+STA 모드 지원
- 별도 사용자 인증 없음
- WebSocket 없음
- 웹 상태 갱신은 polling 기반
- CAN 송신 성공 여부는 HTTP 응답으로만 확인
- MKBD의 최종 상태 판정은 Broadcast 또는 Response 수신 후 확정

## 11. 확장 가능성

현실적인 다음 확장 후보는 다음 정도이다.

- WebSocket 기반 실시간 상태 갱신
- 최근 CAN 로그를 웹 UI에 표시
- 버튼 UI를 실제 Head Unit 패널 형태로 개선
- HU와 MKBD 공통 CAN 상수 헤더 분리

## 12. 면접 설명 요약

HU는 ESP8266이 제공하는 웹 UI를 통해 사용자의 HVAC/INFO 제어 입력을 받고, 이를 공통 CAN Payload 규격에 맞는 Request Frame으로 변환해 MKBD로 전송한다.

CAN Payload 규격의 소유는 MKBD의 공통 CAN 명세에 두고, HU 명세는 웹 UI, Web API, 상태 동기화, CAN 송수신 책임만 설명하도록 분리했다.
