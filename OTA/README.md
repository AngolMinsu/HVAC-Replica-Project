# Local OTA server

HU7과 같은 Wi-Fi에 연결된 PC에서 실행한다.

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
