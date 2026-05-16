# HU32 FreeRTOS HMI

## Required libraries

- TFT_eSPI
- PNGdec
- mcp_can

## Display setup

The target display is ST7796S, 480x320 landscape, `setRotation(1)`.
TFT_eSPI pin settings are managed in the library setup file:

```plain text
C:\Users\djvmf\Documents\Arduino\libraries\TFT_eSPI\User_Setup.h
```

## Asset upload

Runtime assets are loaded from LittleFS:

```plain text
/assets/topbar_base.png
/assets/bottombar_base.png
/assets/media_panel_normal.png
...
```

The files are mirrored under:

```plain text
HU32/data/assets/
```

Use the `Huge APP (3MB No OTA/1MB SPIFFS)` partition scheme and upload the LittleFS data folder before running the sketch.

## RTOS structure

```plain text
CAN RX Task      -> receives CAN frames and emits HuStateEvent
CAN TX Task      -> waits for HuCanCommand and sends CAN requests
Input Task       -> converts input into UiEvent
Asset Task       -> mounts LittleFS and prepares assets
System Task      -> owns SystemState and dirty flags
UI Task          -> owns all TFT drawing
```

Only `UI Task` draws to the TFT. Other tasks communicate through queues.

## Serial test input

```plain text
n : focus next
p : focus previous
s : select focused panel
b : back
h : home
m : media
c : HVAC
```
