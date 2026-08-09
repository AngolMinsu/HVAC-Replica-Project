# Local OTA server

HU7과 같은 Wi-Fi에 연결된 PC에서 실행한다.

## 명세

- [OTA 전체 명세](OTA_SPEC.md)
- [HU7-MKBD CAN OTA 상세 명세](CAN_OTA_MKBD_SPEC.md)

## GUI 실행

`start_ota_manager.bat`을 더블클릭한다.

또는 다음 명령을 실행한다.

```powershell
python ota_manager.py
```

GUI 사용 순서:

1. Target과 Version을 입력한다.
2. `Browse...`에서 빌드된 `.bin`을 선택한다.
3. `Register Firmware`를 누른다.
4. `Start Server`를 누른다.
5. 표시된 HU URL이 HU7의 `OTA_SERVER_BASE_URL`과 같은지 확인한다.
6. HU7 Setting → Connect에서 Target을 선택하고 `Check Update`를 누른다.

GUI가 펌웨어 복사, manifest 생성, 크기와 SHA-256 계산을 처리한다.

### 서버 Binary 관리

`Stored Binary Files` 표에는 target 폴더에 저장된 모든 `.bin`이 표시된다.

- 행 선택: Target, Version, 경로, 크기, SHA-256을 등록 입력란에 불러온다.
- `Set Active`: 선택한 파일을 해당 Target의 배포 Manifest로 지정한다.
- `Delete Selected`: 확인 후 binary를 삭제한다. Active binary이면 Manifest도 함께 삭제한다.
- `Refresh`: target 폴더를 다시 읽는다.
- HU7/MKBD 등록 시 ESP32 application header `0xE9`를 검사한다.
- `.merged.bin`, `.bootloader.bin`, `.partitions.bin`은 등록을 거부한다.

### HU SD Package 관리

HU7의 Setting → Connect → `SD Packages`에서 TF 카드에 저장된 package를 관리한다.

- 저장 경로: `/firmware/HU7/HU7-<version>.bin`, `/firmware/MKBD/MKBD-<version>.bin`
- `Use Package`: SHA-256을 다시 계산하고 Install 대상으로 선택한다.
- `Delete`: 4초 안에 Confirm을 다시 눌러 파일을 삭제한다.
- `Refresh`: TF 카드 package 목록을 다시 읽는다.
- 잘못된 ESP32 image header는 `INVALID`로 표시하며 선택할 수 없다.

## CLI 실행

```powershell
python server.py --host 0.0.0.0 --port 8080
```

Windows 방화벽에서 TCP 8080 인바운드를 허용한다. `ipconfig`로 PC IPv4 주소를 확인하고 HU7의 `OTA_SERVER_BASE_URL`에 `http://PC_IP:8080`을 지정한다.

## HU7 펌웨어 등록

1. 빌드된 `.bin`을 `firmware/HU7/`에 복사한다.
2. `firmware/HU7/manifest.json`의 `version`과 `file`을 수정한다.
3. 서버 재시작은 필요 없다.

서버가 파일 크기와 SHA-256을 직접 계산한다. 존재하지 않는 파일은 배포하지 않는다.

## API

- `GET /api/health`
- `GET /api/firmware/latest?target=HU7`
- `GET /firmware/HU7/<file>`
- `POST /api/ota/result`
