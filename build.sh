 #!/bin/bash

# Set up the environment
cd "$(dirname "$0")"

# The following line would set up Visual Studio environment. This is Windows-specific, so you'd need an equivalent setup for your Linux environment. I'll comment it out.
# source "/path/to/linux/equivalent/of/vcvars64"

source /opt/llvm/setvars.sh

# Create the build directory and navigate to it
mkdir -p build
cd build

# Run the cmake command
cmake -G "Ninja" -D CMAKE_BUILD_TYPE=Release -DOFFLOAD_ARCH=gfx1031 -DSYCL_PATH="/opt/llvm/llvm/build/" -DSYCL_TARGETS=amdgcn-amd-amdhsa ..
if [ $? -ne 0 ]; then
    echo -e "\033[0;31mERROR\033[0m" # Red text
    exit 1
fi

# Run the ninja build
ninja
if [ $? -ne 0 ]; then
    echo -e "\033[0;31mERROR\033[0m" # Red text
    exit 1
fi

# Copy files. Make sure to update paths accordingly for Linux.
cp _deps/oidnzip-src/oidn-1.4.3.x64.vc14.linux/bin/OpenImageDenoise.so ./bin/
if [ $? -ne 0 ]; then
    echo -e "\033[0;31mERROR\033[0m" # Red text
    exit 1
fi

cp _deps/oidnzip-src/oidn-1.4.3.x64.vc14.linux/bin/tbb12.so ./bin/
if [ $? -ne 0 ]; then
    echo -e "\033[0;31mERROR\033[0m" # Red text
    exit 1
fi

# Print success message
echo -e "\033[0;32mSUCCESS\033[0m" # Green text