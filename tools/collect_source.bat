```bat
@echo off
chcp 65001 >nul

cd /d "%~dp0"

echo ========================================
echo Collecting C++ source files...
echo ========================================
echo.

python collect_source.py

echo.
echo ========================================
echo Finished
echo ========================================
pause
```
