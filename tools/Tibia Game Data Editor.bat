@echo off
title Tibia Data Studio - Editor Unificado
cd /d "%~dp0\tibia_data_studio"

echo =========================================================================
echo       TIBIA DATA STUDIO - INICIALIZANDO O EDITOR UNIFICADO
echo =========================================================================
echo.

python run_editor.py

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERRO] Falha ao executar o Python. Verifique se o Python esta instalado no PATH.
    pause
)
