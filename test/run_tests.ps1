$root = Split-Path -Parent $PSScriptRoot

function Invoke-TestBuild {
  param(
    [string]$Name,
    [string]$Output,
    [string[]]$Sources,
    [string[]]$Includes
  )

  $includeArgs = @()
  foreach ($include in $Includes) {
    $includeArgs += "-I$include"
  }

  & g++ -std=c++17 -x c++ @includeArgs @Sources -o $Output
  if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
  }

  & $Output
  if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
  }
}

Invoke-TestBuild `
  -Name "HU" `
  -Output "$PSScriptRoot\HU\test_hu.exe" `
  -Includes @("$PSScriptRoot\TestSupport", "$root\HU") `
  -Sources @(
    "$PSScriptRoot\HU\test_main.cpp",
    "$PSScriptRoot\HU\Test_CanProtocol.cpp",
    "$PSScriptRoot\HU\Test_CommandParser.cpp",
    "$PSScriptRoot\HU\Test_CanDriver.cpp",
    "$PSScriptRoot\HU\Test_CanMonitor.cpp",
    "$PSScriptRoot\HU\Test_WebUi.cpp",
    "$root\HU\CanProtocol.cpp",
    "$root\HU\CommandParser.cpp",
    "$root\HU\CanDriver.cpp",
    "$root\HU\CanMonitor.cpp"
  )

Invoke-TestBuild `
  -Name "MKBD_ROOT" `
  -Output "$PSScriptRoot\MKBD\test_mkbd_root.exe" `
  -Includes @("$PSScriptRoot\TestSupport", "$root\MKBD") `
  -Sources @("$PSScriptRoot\MKBD\test_main.cpp")

Invoke-TestBuild `
  -Name "MKBD_APP" `
  -Output "$PSScriptRoot\MKBD\app\test_mkbd_app.exe" `
  -Includes @("$PSScriptRoot\TestSupport", "$root\MKBD") `
  -Sources @(
    "$PSScriptRoot\MKBD\app\test_main.cpp",
    "$PSScriptRoot\MKBD\app\Test_AppLogic.cpp",
    "$root\MKBD\app\AppLogic.cpp",
    "$root\MKBD\state\State.cpp",
    "$root\MKBD\display\Datc.cpp",
    "$root\MKBD\display\Info.cpp"
  )

Invoke-TestBuild `
  -Name "MKBD_BUTTON" `
  -Output "$PSScriptRoot\MKBD\button\test_mkbd_button.exe" `
  -Includes @("$PSScriptRoot\TestSupport", "$root\MKBD") `
  -Sources @(
    "$PSScriptRoot\MKBD\button\test_main.cpp",
    "$PSScriptRoot\MKBD\button\Test_ButtonInput.cpp",
    "$root\MKBD\button\ButtonInput.cpp"
  )

Invoke-TestBuild `
  -Name "MKBD_STATE" `
  -Output "$PSScriptRoot\MKBD\state\test_mkbd_state.exe" `
  -Includes @("$PSScriptRoot\TestSupport", "$root\MKBD") `
  -Sources @(
    "$PSScriptRoot\MKBD\state\test_main.cpp",
    "$PSScriptRoot\MKBD\state\Test_State.cpp",
    "$root\MKBD\state\State.cpp"
  )

Invoke-TestBuild `
  -Name "MKBD_DISPLAY" `
  -Output "$PSScriptRoot\MKBD\display\test_mkbd_display.exe" `
  -Includes @("$PSScriptRoot\TestSupport", "$root\MKBD") `
  -Sources @(
    "$PSScriptRoot\MKBD\display\test_main.cpp",
    "$PSScriptRoot\MKBD\display\Test_Datc.cpp",
    "$PSScriptRoot\MKBD\display\Test_Info.cpp",
    "$root\MKBD\display\Datc.cpp",
    "$root\MKBD\display\Info.cpp",
    "$root\MKBD\state\State.cpp"
  )

Invoke-TestBuild `
  -Name "MKBD_CAN" `
  -Output "$PSScriptRoot\MKBD\can\test_mkbd_can.exe" `
  -Includes @("$PSScriptRoot\TestSupport", "$root\MKBD") `
  -Sources @(
    "$PSScriptRoot\MKBD\can\test_main.cpp",
    "$PSScriptRoot\MKBD\can\Test_CanProtocol.cpp",
    "$PSScriptRoot\MKBD\can\Test_CanHandler.cpp",
    "$PSScriptRoot\MKBD\can\Test_CanDriver.cpp",
    "$PSScriptRoot\MKBD\can\Test_CanMonitor.cpp",
    "$root\MKBD\can\CanProtocol.cpp",
    "$root\MKBD\can\CanHandler.cpp",
    "$root\MKBD\can\CanDriver.cpp",
    "$root\MKBD\can\CanMonitor.cpp",
    "$root\MKBD\state\State.cpp"
  )

Invoke-TestBuild `
  -Name "INTEGRATION" `
  -Output "$PSScriptRoot\integration\test_integration.exe" `
  -Includes @("$PSScriptRoot\TestSupport", "$root\HU", "$root\MKBD") `
  -Sources @(
    "$PSScriptRoot\integration\test_main.cpp",
    "$PSScriptRoot\integration\Test_EndToEndControlInput.cpp",
    "$root\HU\CommandParser.cpp",
    "$root\MKBD\can\CanProtocol.cpp",
    "$root\MKBD\can\CanHandler.cpp",
    "$root\MKBD\state\State.cpp"
  )

exit 0
