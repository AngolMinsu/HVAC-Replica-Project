# HU7 Review Checklist

- [ ] `GDS.h`와 GW/MKBD32 CAN ID·Signal 일치
- [ ] CAN Task가 LVGL draw를 직접 호출하지 않음
- [ ] UI Task가 TWAI Driver에 직접 접근하지 않음
- [ ] Shared State 접근 보호
- [ ] `src/generated/squareline/` 직접 수정 없음
- [ ] `src/vendor/waveshare_7b/` 직접 수정 없음
- [ ] CAN ID, DLC, Checksum, Signal 실패 로그 확인
- [ ] 화면 전환 후 Top/Bottom Bar 상태 유지
- [ ] build 산출물·대형 Asset 중복 커밋 없음
