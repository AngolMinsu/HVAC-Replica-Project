# HU7-MKBD CAN OTA Software Specification

## 1. 문서 정보

| 항목 | 값 |
|---|---|
| 문서 ID | `OTA-CAN-MKBD-SRS` |
| 문서 버전 | `0.1.0` |
| 프로젝트 적용 버전 | `v0.5` |
| 상태 | 구현 검토용 초안 |
| 작성일 | 2026-08-09 |
| 적용 대상 | HU7, MKBD32, OTA Server |
| 범위 | HU7 → Body CAN → MKBD32 |

이 문서는 HU7이 OTA Server에서 MKBD 펌웨어를 내려받고 Body CAN을 통해 MKBD32를 업데이트하는 기능을 정의한다.

GW 및 BMS CAN OTA는 본 문서 범위에 포함하지 않는다.

프로젝트 기능 버전 기준은 다음과 같다.

- `v0.5`: HU Self OTA와 HU7-MKBD CAN OTA를 포함한 OTA 기능 전체
- `v0.6`: BMS 개발 단계

문서 버전 `0.1.0`은 명세서 자체의 개정 번호이며 프로젝트 기능 버전과 다르다.

## 2. 목표

HU7은 OTA Server와 MKBD 사이의 OTA Coordinator로 동작한다.

```text
OTA Server
    │ Wi-Fi / HTTP
    ▼
HU7
    │ TF Card Staging
    │ Body CAN 500 kbps
    ▼
MKBD32
    │ Inactive OTA Partition
    ▼
Reboot / Version Confirmation
```

정상 완료 조건은 다음과 같다.

1. HU7이 서버 패키지의 Target, Version, Size, SHA-256을 검증한다.
2. HU7이 검증된 펌웨어만 CAN으로 전송한다.
3. MKBD가 비활성 OTA 파티션에 펌웨어를 기록한다.
4. MKBD가 전체 크기, CRC32 및 ESP32 Image 유효성을 검증한다.
5. MKBD가 재부팅한 뒤 새로운 Firmware Version을 응답한다.
6. HU7이 기대 버전과 실행 버전이 같은 경우에만 성공으로 판정한다.

## 3. 범위

### 3.1 포함

- OTA Server의 MKBD Manifest 조회
- MKBD 현재 Firmware Version CAN 조회
- MKBD Firmware `.bin` 다운로드
- TF Card Staging
- SHA-256 검증
- CAN OTA Session 관리
- CAN Firmware 분할 전송
- Block ACK, Timeout, Retry
- MKBD OTA Partition 기록
- CRC32 및 ESP Image 검증
- MKBD 재부팅
- Post-Update Version 확인
- HU7 LVGL 진행률 및 오류 표시
- Serial Log

### 3.2 제외

- GW OTA
- BMS OTA
- STM32 Bootloader
- CAN FD
- 무선 Firmware 서명 및 PKI
- 여러 Node 동시 OTA
- 전송 중단 후 전원 재인가 Resume
- 인터넷 공개 OTA Server 운영

## 4. 현재 시스템 조건

### 4.1 HU7

- MCU: ESP32-S3
- Flash: 16MB
- Storage: TF Card, SD_MMC 1-bit
- Network: Wi-Fi / HTTP
- Vehicle Network: TWAI, Classic CAN 500 kbps
- GUI: LVGL
- RTOS: FreeRTOS

### 4.2 MKBD32

- MCU: ESP32-S3
- Flash: 4MB
- Vehicle Network: TWAI, Classic CAN 500 kbps
- 현재 Firmware Image 크기: 약 368KB
- OTA Partition:

| Partition | Offset | Size |
|---|---:|---:|
| `ota_0` | `0x10000` | `0x140000` |
| `ota_1` | `0x150000` | `0x140000` |

MKBD Firmware Image는 `0x140000`보다 작아야 한다.

MKBD에는 최초 한 번 USB/UART로 CAN OTA Receiver가 포함된 Firmware를 설치해야 한다. 이후부터 CAN OTA를 사용할 수 있다.

## 5. Firmware Package

OTA Server에 등록하는 MKBD Firmware는 다음 파일이다.

```text
MKBD32/build/MKBD32.ino.bin
```

다음 파일은 CAN OTA Package로 사용하지 않는다.

- `MKBD32.ino.merged.bin`
- `MKBD32.ino.bootloader.bin`
- `MKBD32.ino.partitions.bin`

Manifest 예시는 다음과 같다.

```json
{
  "target": "MKBD",
  "version": "0.5.0",
  "file": "MKBD-0.5.0.bin"
}
```

Server API 응답은 최소 다음 값을 제공해야 한다.

```json
{
  "target": "MKBD",
  "version": "0.5.0",
  "file": "MKBD-0.5.0.bin",
  "size": 368448,
  "sha256": "...",
  "url": "/firmware/MKBD/MKBD-0.5.0.bin"
}
```

## 6. Firmware Version 규칙

Firmware Version은 `MAJOR.MINOR.PATCH` 형식을 사용한다.

- `MAJOR`: 0~255
- `MINOR`: 0~255
- `PATCH`: 0~65535
- CAN 응답은 숫자 필드로 전송한다.
- Server Manifest의 문자열 Version은 HU7 Version Manager가 숫자로 변환한다.
- 최신 버전이 현재 버전보다 큰 경우에만 Update를 허용한다.
- Downgrade는 본 버전에서 허용하지 않는다.

## 7. Node 및 Session

### 7.1 Target ID

| Target | 값 |
|---|---:|
| None | `0x00` |
| HU7 | `0x01` |
| MKBD | `0x02` |
| GW | `0x03` |
| BMS | `0x04` |

본 명세에서 MKBD만 OTA 명령을 수락한다.

### 7.2 Session ID

- HU7은 OTA 시작 시 0이 아닌 8-bit Session ID를 생성한다.
- 모든 Control, Data, ACK는 같은 Session ID를 사용한다.
- MKBD는 현재 Session과 다른 Data Frame을 무시한다.
- 한 CAN Bus에서는 하나의 OTA Session만 활성화한다.

## 8. CAN ID

11-bit Standard CAN ID를 사용한다.

| CAN ID | 방향 | 명칭 | 용도 |
|---|---|---|---|
| `0x600` | HU7 → Node | `CAN_OTA_CONTROL` | Version Query, Start, End, Abort |
| `0x601` | HU7 → Node | `CAN_OTA_DATA` | Firmware Data |
| `0x602` | Node → HU7 | `CAN_OTA_FLOW` | Ready, Block ACK, Error |
| `0x603` | Node → HU7 | `CAN_OTA_INFO` | Version, 최종 결과 |

기존 `0x100`, `0x101`, `0x300`과 중복되지 않아야 한다.

## 9. Byte Order와 무결성

- 다중 Byte 정수는 Little Endian을 사용한다.
- CAN Controller의 Frame CRC는 물리 전송 오류 검출에 사용한다.
- Control, Flow, Info Frame의 Byte 7은 Application CRC8이다.
- CRC8 규격은 CRC-8/SAE-J1850을 사용한다.
  - Polynomial: `0x1D`
  - Init: `0xFF`
  - RefIn/RefOut: false
  - XorOut: `0xFF`
- Data Frame은 8 Byte 전체를 사용하며 별도 CRC8을 넣지 않는다.
- 전체 Firmware는 CRC32/ISO-HDLC로 검증한다.
  - Polynomial: `0x04C11DB7`
  - Reflected Polynomial: `0xEDB88320`
  - Init: `0xFFFFFFFF`
  - XorOut: `0xFFFFFFFF`

## 10. CAN Frame 정의

### 10.1 Control Frame — `0x600`

| Byte | 필드 | 설명 |
|---:|---|---|
| 0 | Opcode | Control 명령 |
| 1 | Target | `0x02` = MKBD |
| 2 | Session | OTA Session ID |
| 3~6 | Argument | 32-bit Little Endian |
| 7 | CRC8 | Byte 0~6 CRC8 |

Opcode:

| Opcode | 이름 | Argument |
|---:|---|---|
| `0x01` | `VERSION_QUERY` | 0 |
| `0x02` | `START` | Firmware Size |
| `0x03` | `END` | Firmware CRC32 |
| `0x04` | `ABORT` | Abort Reason |
| `0x05` | `STATUS_QUERY` | 0 |

### 10.2 Data Frame — `0x601`

| Byte | 필드 | 설명 |
|---:|---|---|
| 0 | Session | OTA Session ID |
| 1~3 | Offset | Firmware Byte Offset, 24-bit Little Endian |
| 4~7 | Data | Firmware Data 4 Byte |

마지막 Frame에서 실제 남은 데이터가 4 Byte 미만이면 MKBD는 START에서 받은 전체 Size를 기준으로 필요한 Byte만 기록한다. 나머지 Byte는 0으로 채운다.

Offset 처리 규칙:

- `Offset == ExpectedOffset`: 데이터를 기록한다.
- `Offset < ExpectedOffset`: 재전송된 Frame으로 판단하고 중복 기록하지 않는다.
- `Offset > ExpectedOffset`: 데이터를 기록하지 않고 기대 Offset을 ACK로 반환한다.

### 10.3 Flow Frame — `0x602`

| Byte | 필드 | 설명 |
|---:|---|---|
| 0 | Status | Flow Status |
| 1 | Target | 응답 Node |
| 2 | Session | OTA Session ID |
| 3~5 | Next Offset | 다음에 받아야 할 Offset |
| 6 | Progress/Error | 정상 시 %, 오류 시 Error Code |
| 7 | CRC8 | Byte 0~6 CRC8 |

Status:

| 값 | 이름 | 의미 |
|---:|---|---|
| `0x10` | `READY` | START 수락 및 OTA Partition 준비 완료 |
| `0x11` | `BLOCK_ACK` | Block 수신 완료 |
| `0x12` | `VERIFYING` | 전체 Image 검증 중 |
| `0x13` | `VERIFIED` | Image 검증 및 Commit 완료 |
| `0x1F` | `ERROR` | Error Code 확인 필요 |

### 10.4 Info Frame — `0x603`

Version 응답:

| Byte | 필드 | 설명 |
|---:|---|---|
| 0 | Info Type | `0x20` = VERSION |
| 1 | Target | 응답 Node |
| 2 | Session | Query Session ID |
| 3 | Major | Version Major |
| 4 | Minor | Version Minor |
| 5~6 | Patch | Version Patch, Little Endian |
| 7 | CRC8 | Byte 0~6 CRC8 |

Result 응답:

| Byte | 필드 | 설명 |
|---:|---|---|
| 0 | Info Type | `0x21` = RESULT |
| 1 | Target | 응답 Node |
| 2 | Session | OTA Session ID |
| 3 | Result | Success/Failed/Cancelled |
| 4 | Error Code | 최종 Error |
| 5~6 | Reserved | 0 |
| 7 | CRC8 | Byte 0~6 CRC8 |

## 11. 전송 정책

### 11.1 Block

- 한 Block은 Data Frame 32개로 구성한다.
- 한 Block의 최대 Firmware Data는 128 Byte이다.
- HU7은 Block 전송 후 `BLOCK_ACK`를 기다린다.
- ACK의 `Next Offset`이 HU7의 다음 Offset과 같아야 다음 Block을 전송한다.
- 다른 Offset이 오면 해당 Offset부터 다시 전송한다.

### 11.2 Burst와 CAN 부하

- HU7은 최대 8 Frame을 연속 전송한 후 최소 1 RTOS Tick을 양보한다.
- MKBD TWAI RX Queue는 최소 64 Frame으로 설정한다.
- MKBD CAN RX 처리기는 한 번 실행할 때 Queue에 쌓인 Frame을 제한된 개수만큼 연속 처리한다.
- OTA 중 일반 상태 Broadcast 주기를 낮추거나 일시 중지한다.
- Control Response와 안전 관련 Frame은 OTA Data보다 우선한다.

### 11.3 Timeout과 Retry

| 항목 | 기본값 |
|---|---:|
| Control 응답 Timeout | 500ms |
| Block ACK Timeout | 500ms |
| Control Retry | 3회 |
| Block Retry | 3회 |
| MKBD Session Inactivity Timeout | 3초 |
| MKBD Reboot 대기 | 최대 10초 |
| Post-Check Version Query | 1초 간격, 최대 10회 |

값은 GDS 공용 상수로 정의하며 코드에 매직넘버로 분산하지 않는다.

## 12. HU7 상태 머신

```text
IDLE
  ↓
QUERY_NODE_VERSION
  ↓
CHECK_SERVER_VERSION
  ↓
DOWNLOAD_TO_SD
  ↓
VERIFY_SHA256
  ↓
START_REMOTE_OTA
  ↓
TRANSFER_BLOCKS
  ↓
REMOTE_VERIFY
  ↓
WAIT_NODE_REBOOT
  ↓
POST_CHECK_VERSION
  ├─ 기대 버전 일치 → COMPLETED
  └─ 불일치/Timeout → FAILED
```

HU7은 Firmware 전체를 RAM에 적재하지 않는다. SD File에서 Block 단위로 읽는다.

## 13. MKBD 상태 머신

```text
NORMAL
  ↓ START
PREPARING
  ↓ Update.begin 성공
RECEIVING
  ↓ END
VERIFYING
  ↓ Size/CRC32/Update.end 성공
REBOOT_PENDING
  ↓
REBOOT

모든 실패 → ERROR → 기존 Firmware 유지
```

MKBD가 `RECEIVING` 상태일 때:

- 현재 Output 상태를 유지한다.
- CAN OTA Frame을 우선 처리한다.
- 일반 제어 요청에는 Busy 응답을 보내거나 명시적으로 무시한다.
- 일반 Broadcast는 CAN 대역폭 확보를 위해 감소시킨다.
- Input Scan과 Watchdog 처리는 중단하지 않는다.

## 14. HU7 기능 요구사항

| ID | 요구사항 |
|---|---|
| `CANOTA-HU-001` | HU7은 선택 Target이 MKBD일 때 MKBD Manifest를 조회해야 한다. |
| `CANOTA-HU-002` | HU7은 CAN으로 MKBD 현재 Version을 조회해야 한다. |
| `CANOTA-HU-003` | HU7은 최신 Version이 더 큰 경우에만 Update 버튼을 활성화해야 한다. |
| `CANOTA-HU-004` | HU7은 Firmware를 TF Card `/firmware/MKBD/`에 저장해야 한다. |
| `CANOTA-HU-005` | HU7은 CAN 전송 전 Size와 SHA-256을 검증해야 한다. |
| `CANOTA-HU-006` | HU7은 Firmware 전체를 RAM에 적재하면 안 된다. |
| `CANOTA-HU-007` | HU7은 Target과 Session이 일치하는 ACK만 처리해야 한다. |
| `CANOTA-HU-008` | HU7은 Block Timeout 시 정해진 횟수만 재전송해야 한다. |
| `CANOTA-HU-009` | Retry를 초과하면 ABORT를 전송하고 FAILED로 전환해야 한다. |
| `CANOTA-HU-010` | HU7은 전송 Byte 기준 Progress를 UI에 제공해야 한다. |
| `CANOTA-HU-011` | HU7은 MKBD 재부팅 뒤 실행 Version을 재확인해야 한다. |
| `CANOTA-HU-012` | HU7은 기대 Version과 실행 Version이 같을 때만 완료 처리해야 한다. |
| `CANOTA-HU-013` | OTA 동작은 LVGL UI Task를 Blocking하면 안 된다. |
| `CANOTA-HU-014` | CAN RX Task는 OTA 응답을 OTA 전용 Queue로 전달해야 한다. |

## 15. MKBD 기능 요구사항

| ID | 요구사항 |
|---|---|
| `CANOTA-MKBD-001` | MKBD는 Target이 MKBD인 OTA Control만 수락해야 한다. |
| `CANOTA-MKBD-002` | MKBD는 현재 Firmware Version을 CAN으로 응답해야 한다. |
| `CANOTA-MKBD-003` | MKBD는 Firmware Size가 OTA Slot보다 크면 START를 거부해야 한다. |
| `CANOTA-MKBD-004` | MKBD는 `Update.begin(size)` 성공 후에만 READY를 전송해야 한다. |
| `CANOTA-MKBD-005` | MKBD는 Offset 순서가 맞는 Data만 기록해야 한다. |
| `CANOTA-MKBD-006` | MKBD는 중복 Data를 중복 기록하면 안 된다. |
| `CANOTA-MKBD-007` | MKBD는 Block마다 Next Offset을 ACK해야 한다. |
| `CANOTA-MKBD-008` | MKBD는 전체 Size와 CRC32를 검증해야 한다. |
| `CANOTA-MKBD-009` | MKBD는 ESP32 Firmware Image 검증이 성공한 경우에만 Commit해야 한다. |
| `CANOTA-MKBD-010` | 검증 실패 시 MKBD는 재부팅하지 않아야 한다. |
| `CANOTA-MKBD-011` | 수신 중 전원이 차단돼도 기존 Boot Partition이 실행 가능해야 한다. |
| `CANOTA-MKBD-012` | Session Timeout 시 진행 중인 Update를 중단해야 한다. |
| `CANOTA-MKBD-013` | MKBD는 OTA 실패 원인을 Error Code로 응답해야 한다. |
| `CANOTA-MKBD-014` | MKBD는 OTA 중에도 Input Scan과 Watchdog를 유지해야 한다. |

## 16. 오류 코드

| 값 | 이름 | 발생 위치 |
|---:|---|---|
| `0x00` | `NONE` | - |
| `0x01` | `INVALID_TARGET` | HU/MKBD |
| `0x02` | `INVALID_STATE` | HU/MKBD |
| `0x03` | `SESSION_MISMATCH` | MKBD |
| `0x04` | `OFFSET_MISMATCH` | MKBD |
| `0x05` | `IMAGE_TOO_LARGE` | MKBD |
| `0x06` | `UPDATE_BEGIN_FAILED` | MKBD |
| `0x07` | `FLASH_WRITE_FAILED` | MKBD |
| `0x08` | `CRC32_MISMATCH` | MKBD |
| `0x09` | `INVALID_IMAGE` | MKBD |
| `0x0A` | `TIMEOUT` | HU/MKBD |
| `0x0B` | `CANCELLED` | HU/MKBD |
| `0x0C` | `CAN_TX_FAILED` | HU/MKBD |
| `0x0D` | `VERSION_TIMEOUT` | HU |
| `0x0E` | `SD_ERROR` | HU |
| `0x0F` | `SHA256_MISMATCH` | HU |
| `0x10` | `SERVER_ERROR` | HU |

## 17. UI 요구사항

기존 `SettingConnect` 화면을 사용한다.

Target이 MKBD일 때 표시값:

- Update Target: `MKBD`
- Current Version: CAN 조회 결과
- Latest Version: Server Manifest 결과
- OTA Server: Connected/Disconnected
- Firmware Size
- Target Status: Online/Offline/Updating/Error
- Package Target: `MKBD`
- Progress: 다운로드와 CAN 전송 진행률
- Status Message

진행률 정책:

| 구간 | UI Progress |
|---|---:|
| Server Download | 0~20% |
| SD/SHA-256 검증 | 20~25% |
| CAN 전송 | 25~90% |
| MKBD 검증 | 90~95% |
| Reboot/Post Check | 95~100% |

OTA 실행 중 Target, Check Update, Update 버튼은 비활성화한다.

## 18. 전원 차단 및 복구

### 18.1 CAN 전송 중 전원 차단

- MKBD는 비활성 OTA Partition에 기록한다.
- `Update.end()` 전에는 Boot Partition을 변경하지 않는다.
- 재부팅 후 기존 Firmware가 실행되어야 한다.
- HU7은 이전 Session을 자동 Resume하지 않는다.
- 사용자가 처음부터 다시 Update한다.

### 18.2 검증 완료 후 재부팅 구간 전원 차단

- ESP32 Bootloader가 Commit된 OTA Partition을 실행할 수 있다.
- HU7은 연결 복구 후 Version Query로 결과를 확인한다.
- 자동 Rollback은 후속 명세에서 다룬다.

## 19. 보안 제한

- SHA-256과 CRC32는 무결성 검증이다.
- Firmware 제작자 인증 기능은 아니다.
- 본 단계는 신뢰된 Local OTA Server와 폐쇄된 개발 CAN Bus를 전제로 한다.
- 외부 배포 전에는 Firmware 서명과 Manifest 인증을 추가해야 한다.

## 20. 로그 요구사항

HU7 로그 예시:

```text
CAN_OTA START target:MKBD session:12 size:368448
CAN_OTA BLOCK offset:4096 retry:0
CAN_OTA ACK next:4224 progress:1
CAN_OTA END crc32:12345678
CAN_OTA POST_CHECK expected:0.5.0 running:0.5.0
CAN_OTA RESULT:SUCCESS
```

MKBD 로그 예시:

```text
CAN_OTA RX START session:12 size:368448
CAN_OTA UPDATE_BEGIN:OK
CAN_OTA WRITE offset:4096
CAN_OTA VERIFY:OK
CAN_OTA REBOOT
```

로그에 Wi-Fi 비밀번호, 인증정보 또는 Firmware 원문을 출력하지 않는다.

## 21. 검증 및 인수 기준

| 시험 ID | 시험 | 합격 기준 |
|---|---|---|
| `CANOTA-T-001` | 정상 MKBD Update | 재부팅 후 기대 Version 응답 |
| `CANOTA-T-002` | 동일 Version | Update 버튼 비활성화 |
| `CANOTA-T-003` | 잘못된 Package Target | CAN 전송 시작 전 실패 |
| `CANOTA-T-004` | SHA-256 불일치 | CAN 전송 시작 전 실패 |
| `CANOTA-T-005` | Data Frame 누락 | Next Offset부터 재전송 후 완료 |
| `CANOTA-T-006` | Data Frame 중복 | 중복 Flash Write 없이 완료 |
| `CANOTA-T-007` | Block ACK Timeout | 3회 Retry 후 명시적 실패 |
| `CANOTA-T-008` | CRC32 불일치 | MKBD 재부팅 금지, 기존 Firmware 유지 |
| `CANOTA-T-009` | 전송 중 MKBD 전원 차단 | 기존 Firmware 정상 부팅 |
| `CANOTA-T-010` | 전송 중 HU7 전원 차단 | MKBD Timeout 후 기존 Firmware 유지 |
| `CANOTA-T-011` | MKBD Offline | Version Timeout 표시, Update 금지 |
| `CANOTA-T-012` | CAN Bus-Off | 실패 표시, 자동 무한 재시도 금지 |
| `CANOTA-T-013` | UI 반응성 | OTA 중 Touch/UI Task가 정지하지 않음 |
| `CANOTA-T-014` | Post Check Version 불일치 | 완료가 아닌 FAILED 처리 |

## 22. 파일 변경 계획

구현 승인 후 예상 변경 범위이다.

### 22.1 HU7

| 파일/모듈 | 변경 목적 |
|---|---|
| `HU7/GDS.h` | CAN OTA ID, Timeout, Block 상수 |
| `HU7/src/ota/OtaTypes.h` | Remote OTA 상태와 오류 추가 |
| `HU7/src/ota/OtaManager.cpp` | MKBD Check/Update 흐름 연결 |
| `HU7/src/ota/OtaHttpClient.*` | MKBD Firmware SD 다운로드 지원 |
| `HU7/src/ota/CanOtaProtocol.*` | Frame Encode/Decode, CRC |
| `HU7/src/ota/CanOtaTransport.*` | Session, Block 전송, Retry |
| `HU7/src/task/task10ms/can/CanRxTask.cpp` | OTA ACK 전용 Queue 라우팅 |
| `HU7/src/storage/StorageManager.*` | `/firmware/MKBD` Staging API |
| `HU7/src/hmi/HeadUnitHmi.cpp` | MKBD 상태와 진행률 표시 |

### 22.2 MKBD32

| 파일/모듈 | 변경 목적 |
|---|---|
| `MKBD32/GDS.h` | CAN OTA ID, Timeout, Block 상수 |
| `MKBD32/ota/CanOtaProtocol.*` | Frame Decode/Encode, CRC |
| `MKBD32/ota/CanOtaReceiver.*` | Session과 Offset 관리 |
| `MKBD32/ota/MkbdOtaInstaller.*` | ESP32 Update Partition 기록 |
| `MKBD32/task/task10ms/can/CanRxTask.cpp` | OTA Frame 우선 처리 |
| `MKBD32/can/CanDriver.cpp` | RX Queue 확장 |
| `MKBD32/MkbdBuild.cpp` | 신규 OTA 구현 파일 포함 |

### 22.3 OTA Server

| 경로 | 변경 목적 |
|---|---|
| `OTA/firmware/MKBD/` | MKBD Firmware와 Manifest 보관 |
| `OTA/ota_manager.py` | MKBD Package 등록 및 조회 확인 |

## 23. 구현 단계

1. 공용 CAN OTA Protocol과 Host Unit Test 작성
2. MKBD Version Query/Response 구현
3. MKBD OTA Receiver와 Update Writer 구현
4. MKBD USB Bootstrap Firmware 빌드 및 설치
5. HU7 ACK Queue와 CAN OTA Transport 구현
6. HU7 SD Staging 및 SHA-256 검증 구현
7. HU7 OtaManager와 LVGL 연결
8. 정상 전송 실기 시험
9. Frame Drop, Timeout, CRC 오류 시험
10. 전원 차단 복구 시험

## 24. 구현 승인 조건

다음 항목을 검토한 후 코드를 수정한다.

- CAN ID `0x600~0x603` 사용 승인
- Block 크기 32 Frame 승인
- 최초 MKBD USB Bootstrap 승인
- OTA 중 일반 Broadcast 감소 정책 승인
- Resume 및 자동 Rollback 제외 승인

READY FOR IMPLEMENTATION REVIEW
