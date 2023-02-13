@setlocal enableextensions
@cd /d "%~dp0"

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
mkdir build
cd build
cmake -G "Ninja" -D CMAKE_BUILD_TYPE=Release -DBOOST_ALL_NO_LIB=ON ..\ 
ninja
copy _deps\sycl_llvm-src\build\bin\sycl.dll .\bin\
copy _deps\oidnzip-src\oidn-1.4.3.x64.vc14.windows\bin\OpenImageDenoise.dll .\bin\
copy _deps\oidnzip-src\oidn-1.4.3.x64.vc14.windows\bin\tbb12.dll .\bin\
pause