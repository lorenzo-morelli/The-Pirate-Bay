@echo off
setlocal enabledelayedexpansion

for %%F in (*.frag) do (
    set "filename=%%~nF"
    set "outputname=!filename:Shader=!"
    glslc "%%F" -o "!outputname!Frag.spv"
)

for %%F in (*.vert) do (
    set "filename=%%~nF"
    set "outputname=!filename:Shader=!"
    glslc "%%F" -o "!outputname!Vert.spv"
)

endlocal