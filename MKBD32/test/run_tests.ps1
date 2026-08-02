$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$build = Join-Path $PSScriptRoot "build"
New-Item -ItemType Directory -Force $build | Out-Null
& g++ -std=c++17 -Wall -Wextra -Werror "-I$(Join-Path $PSScriptRoot 'TestSupport')" `
  (Join-Path $PSScriptRoot "Test_MKBD.cpp") `
  (Join-Path $root "can/CanProtocol.cpp") `
  (Join-Path $root "state/State.cpp") `
  -o (Join-Path $build "test_mkbd.exe")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& (Join-Path $build "test_mkbd.exe")
exit $LASTEXITCODE
