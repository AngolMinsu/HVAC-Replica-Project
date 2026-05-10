# Unit Test Structure

이 폴더는 `HU`, `MKBD` 원본 구조를 따라 만든 PC 실행용 단위 테스트 영역입니다.

## 구조

- `TestSupport/`: PC 컴파일을 위한 최소 Arduino/U8g2/MCP2515 스텁과 ASSERT 매크로
- `HU/`: HU 원본 루트 구조에 대응하는 테스트 파일과 `test_main.cpp`
- `MKBD/`: MKBD 원본 루트 구조에 대응하는 테스트 파일과 `test_main.cpp`
- `MKBD/app/`: `AppLogic` 테스트
- `MKBD/button/`: `ButtonInput` 테스트
- `MKBD/can/`: `CanProtocol`, `CanHandler`, `CanDriver`, `CanMonitor` 테스트
- `MKBD/display/`: `Datc`, `Info` 테스트
- `MKBD/state/`: `State` 테스트

## 실행

전체 테스트는 `TestSupport/Test_Assert.cpp` 하나만 빌드하고 실행하면 됩니다.

```powershell
g++ -std=c++17 -Itest\TestSupport test\TestSupport\Test_Assert.cpp -o test\test_all.exe
.\test\test_all.exe
```

반환값은 아래 규칙을 따릅니다.

- `return 0`: OK
- `return 1`: NOK

폴더별 테스트를 직접 실행하고 싶을 때는 아래 스크립트를 사용할 수 있습니다.

```powershell
.\test\run_tests.ps1
```

테스트는 각 폴더의 `test_main.cpp`에서 `ASSERT(Test_...(loop, input..., actual..., expected...))` 형태로 호출하고,
각 테스트 파일 내부에서 `ASSERT_EQUALS(index, actual, expected)`로 필드 단위 값을 비교합니다.
불일치하면 파일, 줄 번호, actual/expected 값이 터미널에 출력되고 테스트가 중단됩니다.

`loop`는 같은 입력 시나리오를 반복 실행하기 위한 첫 번째 인자입니다. 예를 들어 엔코더를 3회 돌려야 다음 상태로 넘어가는 요구사항이 생기면 `ASSERT(Test_Encoder(3, ...))` 형태로 반복 검증할 수 있습니다.

테스트 케이스는 최소값, 최대값, 경계값, 중앙값, 요구사항 대표값을 포함하도록 구성했습니다.
