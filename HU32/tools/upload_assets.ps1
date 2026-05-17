$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$dataDir = Join-Path $projectRoot "data"
$buildDir = Join-Path $projectRoot "build"
$imagePath = Join-Path $buildDir "littlefs.bin"

$mklittlefs = "C:\Users\djvmf\AppData\Local\Arduino15\packages\esp32\tools\mklittlefs\3.0.0-gnu12-dc7f933\mklittlefs.exe"
$esptool = "C:\Users\djvmf\AppData\Local\Arduino15\packages\esp32\tools\esptool_py\4.5.1\esptool.exe"

$port = if ($args.Count -gt 0) { $args[0] } else { "COM10" }

New-Item -ItemType Directory -Force $buildDir | Out-Null

Write-Host "[ASSET] Build LittleFS image..."
& $mklittlefs -c $dataDir -p 256 -b 4096 -s 0xE0000 $imagePath

Write-Host "[ASSET] Upload LittleFS image to $port..."
& $esptool --chip esp32s3 --port $port --baud 921600 write_flash 0x310000 $imagePath

Write-Host "[ASSET] Done."
