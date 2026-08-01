# HU7 작업 규칙

## 범위

- Waveshare ESP32-S3 Touch LCD 7B 기반 Head Unit.
- 역할: CAN 수신, HMI 상태 반영, LVGL 화면 전환, 터치 입력 처리.
- MKBD와 Gateway 소스는 이 폴더에 추가하지 않는다.

## 직접 수정

- `GDS.h`: CAN ID, Signal, Task 주기, Head Unit 공용 설정
- `HU7.ino`: 초기화와 Task 시작점
- `src/can/`: TWAI CAN Driver, Protocol, Monitor
- `src/hmi/`: HMI 상태와 화면 동기화
- `src/task/`: CAN RX, Input, UI Task
- `src/driver/`: DisplayDriver 인터페이스
- `docs/`, `assets/`, `SSUI/`: 문서, UI 원본, 이미지

## 생성 코드 및 Vendor

- `src/generated/squareline/`: 직접 수정 금지. `SSUI/`에서 수정 후 재생성.
- `src/vendor/waveshare_7b/`: 보드 제공 드라이버. 직접 수정 금지.
- Vendor 수정이 필요한 경우 별도 Wrapper를 `src/driver/`에 추가하고 원인과 변경점을 문서화한다.

## 실행 구조

- CAN RX Task: 10ms, 수신 프레임 Queue 전달
- Input Task: 10ms, 터치 입력을 UI Event로 변환
- UI Task: 20ms, 상태 변경 영역만 LVGL 갱신
- UI Task는 CAN Driver에 직접 접근하지 않는다.
- CAN Task는 LVGL draw를 직접 호출하지 않는다.

## CAN 규칙

- CAN ID, Service, Result, Signal은 `GDS.h`를 기준으로 한다.
- DLC 8, D7은 D0~D6 XOR Checksum.
- `0x100` Request, `0x101` Response/ACK, `0x300` HVAC Status Broadcast.
- Gateway ACK는 Gateway 도착 확인이다. 실제 반영은 Response로 판단한다.

## 검증

- 화면 출력, 터치, CAN 수신, 상태 반영을 분리 확인한다.
- CAN 오류 시 ID, DLC, Checksum, Signal, TWAI 상태를 기록한다.
- `build/`, 생성 이미지 C 배열, IDE 캐시는 수정/커밋하지 않는다.## Commit Message

형식: `<type>: <변경 단위 요약>`

- `init`: 최초 구조 생성
- `add`: 파일, 모듈, 문서 추가
- `modify`: 일반 변경
- `update`: 버전 업, 구조 개편, 주요 기능 확장
- `remove`: 파일, 코드, 기능 삭제
- `feat`: 신규 기능
- `fix`: 결함, 빌드, 통신 오류 수정
- `refactor`: 동작 변경 없는 구조 정리
- `test`: Unit Test, Integration Test 변경
- `review`: 검증 및 리뷰 결과
- `docs`: 명세, 하네스, README 변경
- `config`: GPIO, Board, IDE 설정
- `build`: 빌드 설정, 의존성 변경
- `chore`: 유지보수
- `revert`: 이전 변경 복구

- 한 커밋에는 하나의 논리적 변경만 포함한다.
- 빌드 가능한 상태에서만 코드 커밋한다.
- 빌드 산출물은 커밋하지 않는다.
