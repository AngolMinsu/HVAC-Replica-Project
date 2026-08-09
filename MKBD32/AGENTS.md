# MKBD32 작업 규칙

## 범위

- ESP32-S3 기반 MKBD 입력 노드.
- 역할: 버튼·로터리 엔코더 입력, DATC/INFO 상태 변경, OLED/Fan 출력, CAN 상태 Broadcast.
- Head Unit과 Gateway 소스는 이 폴더에 추가하지 않는다.

## 직접 수정

- `GDS.h`: GPIO, CAN ID, Signal, 상태 범위, PWM 정책
- `MKBD32.ino`: Arduino 시작점
- `MkbdApp.*`, `MkbBuild.cpp`: 애플리케이션 조립과 흐름
- `app/`, `button/`, `encoder/`, `state/`, `display/`, `can/`, `task/`: 기능 모듈
- `docs/`: 하네스, 명세, 검증 문서

## 실행 구조

- `task/task10ms/input/`: 버튼·엔코더 입력 해석
- `task/task10ms/can/`: TWAI 수신 처리
- `task/task10ms/output/`: Fan 등 출력 반영
- `task/task100ms/display/`: OLED 갱신
- `MkbRtos.*`: Task 생성과 주기 관리
- 입력, 상태 변경, CAN 송신, 출력 처리를 섞지 않는다.

## CAN 규칙

- CAN ID, Service, Result, Signal은 `GDS.h`를 기준으로 한다.
- DLC 8, D7은 D0~D6 XOR Checksum.
- `0x100` Request, `0x101` Response/ACK, `0x300` HVAC Status Broadcast.
- 현재 `TWAI_MODE_NO_ACK`는 단독 벤치 확인용이다. 3노드 통합 시 `TWAI_MODE_NORMAL` 사용 여부를 검토한다.

## 검증

- 엔코더 A/B 접점, Button Switch, OLED I2C, CAN Bus를 분리 확인한다.
- TWAI 오류 시 `state`, `txErr`, `rxErr`, `busErr`를 확인한다.
- `build/`, `.vscode/`, IDE 캐시, 로그는 커밋하지 않는다.## Commit Message

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
