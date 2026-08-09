$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$build = Join-Path $PSScriptRoot "build"
New-Item -ItemType Directory -Force $build | Out-Null
& g++ -std=c++17 -Wall -Wextra -Werror "-I$(Join-Path $PSScriptRoot 'TestSupport')" `
  (Join-Path $PSScriptRoot "test_main.cpp") `
  (Join-Path $PSScriptRoot "Test_MKBD.cpp") `
  (Join-Path $PSScriptRoot "TestSupport/Test_Stubs.cpp") `
  (Join-Path $root "can/CanProtocol.cpp") `
  (Join-Path $root "state/State.cpp") `
  (Join-Path $root "button/ButtonInput.cpp") `
  (Join-Path $root "encoder/EncoderInput.cpp") `
  (Join-Path $root "display/Datc.cpp") `
  (Join-Path $root "display/Info.cpp") `
  (Join-Path $root "app/AppLogic.cpp") `
  (Join-Path $root "app/MkbdHardware.cpp") `
  (Join-Path $root "can/CanHandler.cpp") `
  (Join-Path $root "can/CanDriver.cpp") `
  (Join-Path $root "can/CanMonitor.cpp") `
  (Join-Path $root "can/MkbdCanService.cpp") `
  (Join-Path $root "task/MkbdRtos.cpp") `
  (Join-Path $root "task/task10ms/can/CanRxTask.cpp") `
  (Join-Path $root "task/task10ms/input/InputTask.cpp") `
  (Join-Path $root "task/task10ms/output/OutputTask.cpp") `
  (Join-Path $root "task/task100ms/display/DisplayTask.cpp") `
  -o (Join-Path $build "test_mkbd.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& (Join-Path $build "test_mkbd.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& g++ -std=c++17 -Wall -Wextra -Werror "-I$(Join-Path $PSScriptRoot 'TestSupport')" `
  (Join-Path $PSScriptRoot "Test_AppAssembly.cpp") `
  (Join-Path $PSScriptRoot "TestSupport/Test_Stubs.cpp") `
  (Join-Path $root "MkbdBuild.cpp") `
  (Join-Path $root "MkbdApp.cpp") `
  -o (Join-Path $build "test_mkbd_app.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& (Join-Path $build "test_mkbd_app.exe")
exit $LASTEXITCODE
