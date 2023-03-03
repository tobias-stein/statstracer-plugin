set sourceDir=%CD%\
set outputDir=%CD%\build

::rmdir /S /Q %sourceDir%
rmdir /S /Q %outputDir%

::git clone https://steint@bitbucket.org/steint/statstracer-ue4-plugin.git %sourceDir%                

::call StatsTracerBuild.bat %sourceDir% %outputDir% 4.18
::call StatsTracerBuild.bat %sourceDir% %outputDir% 4.19
::call StatsTracerBuild.bat %sourceDir% %outputDir% 4.20
::call StatsTracerBuild.bat %sourceDir% %outputDir% 4.21
::call StatsTracerBuild.bat %sourceDir% %outputDir% 4.22
call StatsTracerBuild.bat %sourceDir% %outputDir% 4.24
