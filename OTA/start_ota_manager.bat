@echo off
cd /d "%~dp0"
where pythonw >nul 2>nul
if errorlevel 1 (
    python ota_manager.py
) else (
    start "" pythonw ota_manager.py
)
