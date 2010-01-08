@echo OFF
call bldmake -k bldfiles
call abld build gcce urel
call bldmake -k bldfiles
call abld export
pause