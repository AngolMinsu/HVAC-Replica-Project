# HU7 Waveshare 7B Port

HU7 uses the Waveshare ESP32-S3-Touch-LCD-7B LCD/Touch/LVGL port.

Required official demo files:

```text
examples/13_LVGL_TRANSPLANT/lvgl_port.cpp
examples/13_LVGL_TRANSPLANT/lvgl_port.h
examples/13_LVGL_TRANSPLANT/rgb_lcd_port.cpp
examples/13_LVGL_TRANSPLANT/rgb_lcd_port.h
examples/13_LVGL_TRANSPLANT/touch.cpp
examples/13_LVGL_TRANSPLANT/touch.h
examples/13_LVGL_TRANSPLANT/gt911.cpp
examples/13_LVGL_TRANSPLANT/gt911.h
examples/13_LVGL_TRANSPLANT/i2c.cpp
examples/13_LVGL_TRANSPLANT/i2c.h
examples/13_LVGL_TRANSPLANT/io_extension.cpp
examples/13_LVGL_TRANSPLANT/io_extension.h
```

They are copied into:

```text
HVAC Replica/HU7/src/vendor/waveshare_7b
```

The 7B demo uses direct RGB LCD and GT911 touch code. It does not use the older `ESP32_Display_Panel` `Board()` abstraction.

```text
LCD: 1024 x 600 RGB
Touch: GT911
LVGL: 8.x, 16-bit color, LV_COLOR_16_SWAP 0
```

This project contains the SquareLine export at:

```text
src/generated/squareline
```

Project-owned code:

```text
src/can/CanDriver.*
src/can/CanMonitor.*
src/can/CanProtocol.*
src/driver/DisplayDriver.*
src/hmi/HeadUnitApp.*
src/hmi/HeadUnitHmi.*
src/task/task10ms/can/CanRxTask.*
src/task/task10ms/can/CanTxTask.*
src/task/task10ms/input/InputTask.*
src/task/task20ms/ui/UiTask.*
```

App flow:

```text
HU7.ino
-> headUnitAppBegin()
-> displayDriverBegin()
-> touch_gt911_init()
-> waveshare_esp32_s3_rgb_lcd_init()
-> lvgl_port_init()
-> ui_init()
-> canDriverBegin()
-> xTaskCreatePinnedToCore(CAN_RX / CAN_TX / INPUT / UI)

loop()
-> vTaskDelay()

UI task
-> headUnitHmiTick()
-> displayDriverLoop()
```

Note:

```text
Current CAN driver is a structural placeholder.
Actual 7B CAN pin and protocol connection should be implemented in src/can.
```

Board setting:

```text
Board: ESP32S3 Dev Module
Flash: 16MB
PSRAM: OPI PSRAM / 8MB
```
