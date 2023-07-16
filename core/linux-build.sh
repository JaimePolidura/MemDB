# Delete initial unused files
rm -r libs/googletest
rm -r tst
rm -r playground
rm -r cmake-build-debug-coverage
rm -r cmake-build-debug
rm -r cmake-build-visualstudio-debug-coverage
rm -r .idea

cd src || exit

#rm -r build

# Calling cmkae
cmake -S .. -O ./build -D BUILDING_LINUX_DEPLOYMENT=ON

# Delete main method in CMakeCXXCompilerId.cpp in that way there is no second definitions
CMAKE_VERSION=$(cmake --version | grep -Eo '[0-9]+\.[0-9]+\.[0-9]+')
cd ./build/CMakeFiles/"$CMAKE_VERSION"

echo "hola $(pwd)" 

if [ -d "./CompilerIdCXX" ]; then
    cd CompilerIdCXX
    LINE_TO_START_REMOVING=$(grep -n 'main' CMakeCXXCompilerId.cpp | head -n 1 | cut -d : -f 1)
    sed -i "${LINE_TO_START_REMOVING},\$d" CMakeCXXCompilerId.cpp
    cd ..
    echo "CompilerIdCXX"
fi

if [ -d "./CompilerIdC" ]; then
    cd CompilerIdC
    LINE_TO_START_REMOVING2=$(grep -n 'main' CMakeCCompilerId.c | head -n 1 | cut -d : -f 1)
    sed -i "${LINE_TO_START_REMOVING2},\$d" CMakeCCompilerId.c
    cd ..
    echo "CompilerIdC"
fi

# Calling make
cd ../..
make -j$(nproc)

echo "BUILD COMPLETED"
