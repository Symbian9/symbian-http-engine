@echo OFF
call bldmake -k bldfiles
call abld build WINSCW udeb
call bldmake -k bldfiles
call abld export
pause