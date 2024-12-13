

@echo off

:: # Check to make sure nrfutil is installed before moving on
WHERE >nul 2>nul nrfutil
IF %ERRORLEVEL% NEQ 0 (
ECHO "nrfutil was not found in PATH, please install using pip install"
goto :end
)

SET APP=SwivX_V2.hex

echo "## Looking to make sure %APP% is present in folder"
if not exist %APP% (
echo "#### app.hex file does not exist! Please copy a application .hex file into the folder, rename it, and try again!"
goto :end
)
echo.

echo "## Creating a FW.zip package that can be used to update the only application FW on the DK"
nrfutil pkg generate --application %APP% --application-version 2 --application-version-string "1.0.1" --hw-version 52 --sd-req 0xCB --key-file private.key FW.zip
echo.

:end
pause