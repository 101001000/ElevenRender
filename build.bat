call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake -G "Ninja" -D CMAKE_BUILD_TYPE=Release . 
ninja
pause