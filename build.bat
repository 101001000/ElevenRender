call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
mkdir build
cd build
cmake -G "Ninja" -D CMAKE_BUILD_TYPE=Release ..\ 
ninja
copy _deps\sycl_llvm-src\build\bin\sycl6.dll .\bin\
copy _deps\oidnzip-src\oidn-1.4.3.x64.vc14.windows\bin\OpenImageDenoise.dll .\bin\
copy _deps\oidnzip-src\oidn-1.4.3.x64.vc14.windows\bin\tbb12.dll .\bin\
pause