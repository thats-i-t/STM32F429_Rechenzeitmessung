:: INFO
:: 1) The make.bat must be placed in the project path.
:: 2) Open cmd (Eingabeaufforderung) in the project path 
:: 2) Call make.bat (optionally with arguments)
:: 2) make.bat calls make (= Tool GNUmake, which is "installed" in another directory)
::    from the project path
:: 3) Make searches for the makefile in the project path.
:: 4) Build process starts

::@echo off
::cls

@echo.
@echo.
@echo ----------------------------------------------------------------------
@echo Started make.bat -- %DATE% %TIME%
@echo ----------------------------------------------------------------------

@SET CURR_PATH=%~dp0
@SET PATH=%CURR_PATH%\tools\GNU_MCU_Eclipse\2.12-20190422-1053\bin

@%PATH%\make %1