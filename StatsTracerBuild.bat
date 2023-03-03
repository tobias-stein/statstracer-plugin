set sourceDir=%1
set outputDir=%2
set ue4_version=%3
set packageDir=%outputDir%\%ue4_version%

call "C:\Program Files\Epic Games\UE_%ue4_version%\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin -Plugin="%sourceDir%\StatsTracer.uplugin" -Package="%packageDir%" -Rocket -targetplatforms=Win32+Win64

if %ERRORLEVEL% EQU 0 (

	xcopy /S /I "%sourceDir%\Config" "%packageDir%\Config"

	pushd %packageDir%
        zip -r "../../StatsTracer-%ue4_version%.zip" * -i Documentation/** Resources/** Config/** Source/**\* StatsTracer.uplugin    
	popd    
)                       