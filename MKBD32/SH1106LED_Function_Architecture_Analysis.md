# 📘 SH1106LED DATC / INFO 구조 분석

## 개요

기존 단일 파일 기반 HVAC 프로젝트를  
DATC(공조) / INFO(인포테인먼트) 구조로 분리하였다.

핵심 목표는:

```text
화면(UI)과 시스템 동작을 분리
```

하는 것이다.

즉:

- INFO 화면으로 전환되어도
- HVAC 시스템은 계속 동작해야 한다.

---

# 📦 전체 구조

```text
MKBD.ino
 ├─ 상태(State) 관리
 ├─ 버튼 입력 처리
 ├─ 화면 전환
 ├─ 팬/LED 하드웨어 제어
 └─ 각 모듈 호출

Datc.cpp
 ├─ 공조 기능 처리
 └─ 공조 화면 출력

Info.cpp
 ├─ 인포 기능 처리
 └─ 인포 화면 출력

State.cpp
 ├─ 공통 상태 저장
 └─ 문자열 변환 함수
```

---

# 📌 SystemState 구조체

파일:
```text
State.h
```

## 목적

DATC와 INFO가 공유하는 상태 저장소 역할.

```cpp
struct SystemState {
  ScreenMode screenMode;

  HvacMode hvacMode;
  int fanSpeed;
  int setTemp;
  WindMode windMode;

  int volume;
  bool mediaReady;
  bool mapReady;
};
```

---

## 핵심 설계 포인트

화면이 바뀌어도:

```text
fanSpeed
hvacMode
setTemp
```

값이 유지된다.

즉:

```text
UI 상태 ≠ 시스템 상태
```

를 구현한 구조.

---

# 📌 initSystemState()

파일:
```text
State.cpp
```

## 역할

시스템 초기 상태 설정.

```cpp
void initSystemState(SystemState& state)
```

---

## 초기값

| 변수 | 값 |
|---|---|
| screenMode | DATC |
| hvacMode | OFF |
| fanSpeed | 0 |
| setTemp | 24 |
| volume | 10 |

---

## 목적

시스템 시작 시 안정된 기본 상태 보장.

---

# 📌 toggleScreenMode()

## 역할

DATC ↔ INFO 화면 전환.

```cpp
void toggleScreenMode(SystemState& state)
```

---

## 동작

```cpp
state.screenMode =
  (state.screenMode == SCREEN_DATC)
  ? SCREEN_INFO
  : SCREEN_DATC;
```

---

## 핵심 포인트

화면만 바뀐다.

```text
HVAC 상태는 유지됨
```

즉:

- INFO 화면이어도 팬은 계속 돈다.
- 공조 시스템은 중단되지 않는다.

---

# 📌 datcHandleButton2()

파일:
```text
Datc.cpp
```

## 역할

HVAC 모드 변경.

```text
OFF → AC → HEAT → AUTO
```

---

## 구조

```cpp
state.hvacMode = (HvacMode)nextMode;
```

---

## 추가 로직

### OFF 진입 시

```cpp
state.fanSpeed = 0;
```

시스템 정지 상태 표현.

---

### AC/HEAT/AUTO 진입 시

```cpp
if (fanSpeed == 0)
  fanSpeed = 1;
```

최소 송풍 자동 보장.

---

## 설계 의도

실제 자동차 HVAC 구조 모방.

---

# 📌 datcHandleButton3()

## 역할

FAN 속도 변경.

```cpp
state.fanSpeed++;
```

---

## 범위

```text
0 ~ 8
```

---

## 순환 구조

```cpp
if (state.fanSpeed > 8)
  state.fanSpeed = 0;
```

---

# 📌 datcHandleButton4()

## 역할

설정 온도 증가.

```cpp
state.setTemp++;
```

---

## 범위

```text
18 ~ 30℃
```

---

# 📌 datcHandleButton5()

## 역할

풍향 변경.

```text
FACE → FOOT → DEF → MIX
```

---

## 구조

```cpp
state.windMode = (WindMode)nextWind;
```

---

# 📌 datcDrawScreen()

## 역할

DATC 화면 렌더링.

```cpp
void datcDrawScreen(...)
```

---

## 출력 요소

- HVAC MODE
- SET TEMP
- FAN
- AIR MODE
- FAN BAR

---

## 핵심 포인트

단순 텍스트 출력이 아니라:

```text
상태 기반 UI 렌더링
```

구조로 설계됨.

---

# 📌 drawFanBar()

## 역할

FAN 속도를 그래픽 바 형태로 표현.

---

## 구조

```cpp
display.drawBox(...)
display.drawFrame(...)
```

---

## 개념

```text
값 → 시각적 UI 요소 변환
```

---

# 📌 infoHandleButton2()

파일:
```text
Info.cpp
```

## 역할

MEDIA 기능 (추후 개발 예정)

현재는:

```cpp
state.mediaReady = !state.mediaReady;
```

로 상태만 토글.

---

# 📌 infoHandleButton3()

## 역할

볼륨 증가.

```cpp
state.volume++;
```

---

## 범위

```text
0 ~ 30
```

---

# 📌 infoHandleButton4()

## 역할

볼륨 감소.

```cpp
state.volume--;
```

---

# 📌 infoHandleButton5()

## 역할

MAP 기능 (추후 개발 예정)

현재는 상태 토글만 구현.

---

# 📌 infoDrawScreen()

## 역할

INFO 화면 출력.

---

## 출력 요소

- MEDIA 상태
- VOLUME
- VOLUME BAR
- MAP 상태

---

## 설계 포인트

공조 화면과 완전히 독립된 UI 구조.

---

# 📌 updateFanMotor()

파일:
```text
MKBD.ino
```

## 역할

실제 팬 PWM 제어.

```cpp
analogWrite(FAN_MOTOR, pwmValue);
```

---

## 핵심 로직

```cpp
if (state.hvacMode == HVAC_OFF ||
    state.fanSpeed == 0)
```

이면 팬 OFF.

---

## PWM 변환

```cpp
map(state.fanSpeed, 1, 8, 90, 255);
```

---

## 중요한 이유

2핀 DC 팬은:

- 낮은 PWM에서 멈춤
- 떨림 발생 가능

그래서 최소 PWM 보장 필요.

---

# 📌 updateStatusLED()

## 역할

HVAC 상태 LED 제어.

---

## 구조

```cpp
HVAC_OFF → LED OFF
그 외 → LED ON
```

---

# 📌 drawCurrentScreen()

## 역할

현재 화면 모드에 따라
DATC 또는 INFO 화면 호출.

```cpp
if (state.screenMode == SCREEN_DATC)
```

---

## 핵심 포인트

```text
UI Layer 분리
```

실현.

---

# 📌 readButtons()

## 역할

모든 버튼 입력 처리.

---

## 핵심 구조

```cpp
if (state.screenMode == SCREEN_DATC)
```

현재 화면 모드에 따라:

- DATC 함수 호출
- INFO 함수 호출

분기 처리.

---

# 📌 가장 중요한 설계 포인트

## 기존 구조

```text
버튼 → OLED 출력
```

---

## 현재 구조

```text
입력
 ↓
상태 변경
 ↓
기능 처리
 ↓
하드웨어 제어
 ↓
화면 렌더링
```

---

# 📌 최종 정리

이 프로젝트는 단순한 OLED 출력이 아니라:

```text
상태 기반 시스템 설계
```

를 목표로 한다.

핵심 구현 요소:

- UI 분리
- 상태 저장 구조
- 공조/인포 기능 독립화
- 화면 전환
- PWM 기반 팬 제어
- 모듈 분리(.h/.cpp)

이를 통해:

```text
소형 차량 HVAC + 인포테인먼트 구조
```

를 Arduino 환경에서 구현하였다.
