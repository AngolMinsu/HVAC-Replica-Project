# SH1106LED DATC / INFO Modular Code

## 파일 구성

```text
SH1106LED_Datc_Info_Modular.ino
State.h
State.cpp
Datc.h
Datc.cpp
Info.h
Info.cpp
```

## 버튼 기능

### 공통
| 핀 | 기능 |
|---|---|
| D6 | DATC <-> INFO 화면 전환 |

### DATC 화면
| 핀 | 기능 |
|---|---|
| D2 | OFF -> AC -> HEAT -> AUTO |
| D3 | FAN 0~8 |
| D4 | TEMP 18~30 |
| D5 | AIR FACE/FOOT/DEF/MIX |

### INFO 화면
| 핀 | 기능 |
|---|---|
| D2 | MEDIA, 추후 개발 예정 |
| D3 | VOLUME UP |
| D4 | VOLUME DOWN |
| D5 | MAP, 추후 개발 예정 |

## 핵심 설계

화면 모드와 공조 동작을 분리했다.

```text
screenMode = DATC / INFO
hvacMode   = OFF / AC / HEAT / AUTO
```

INFO 화면으로 전환되어도 `updateFanMotor()`는 계속 실행되므로 공조 장치는 유지된다.

## 회로 요약

```text
D2 ---- 버튼 ---- GND
D3 ---- 버튼 ---- GND
D4 ---- 버튼 ---- GND
D5 ---- 버튼 ---- GND
D6 ---- 버튼 ---- GND

D7 ---- 저항 ---- LED+ / LED- ---- GND

D9 ---- 1kΩ ---- 2N2222A Base
2N2222A Collector ---- 팬 -
2N2222A Emitter ------- GND
5V -------------------- 팬 +

OLED SDA -------------- SDA
OLED SCL -------------- SCL
OLED VCC -------------- 5V
OLED GND -------------- GND
```
