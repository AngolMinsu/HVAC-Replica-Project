# MKBD32 Review Checklist

- [ ] `GDS.h`와 GW/HU7 CAN ID·Signal 일치
- [ ] Input, CAN, Output, Display Task 책임 분리
- [ ] 엔코더 A/B 1개 detent 기준 방향 검증
- [ ] Button debounce와 Falling Edge 검증
- [ ] OLED I2C 오류가 CAN 처리 지연을 만들지 않음
- [ ] CAN ID, DLC, Checksum, Signal 실패 로그 확인
- [ ] 3노드 CAN Bus에서 `TWAI_MODE_NORMAL` 송신·수신과 Bus ACK 확인
- [ ] build 산출물·IDE 캐시 커밋 없음
