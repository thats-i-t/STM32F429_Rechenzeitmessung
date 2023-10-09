::set PIP=C:\PyInstaller-3.4\
::pyi-makespec --onefile --console dfu.py
::pyinstaller dfu.spec
pyinstaller --onefile dfu.py

pause
