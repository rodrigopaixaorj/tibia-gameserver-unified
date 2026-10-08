@echo off
echo ==============================================================================
echo Tibia 7.72 - SQLite to MySQL Exporter
echo ==============================================================================
echo.
cd /d "%~dp0\.."
python tools\export_sqlite_to_mysql.py tibia.db tibia_mysql_full.sql
echo.
echo Done! The file tibia_mysql_full.sql is ready to be imported into MySQL/MariaDB.
pause
