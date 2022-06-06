to configure and build with cmake:

- if it does not exist yet, create a build directory and change to it:

mkdir build
cd build

- from there configure and build using cmake:

cmake .. -G "Visual Studio 17 2022" -T host=x64 -A x64
cmake --build .

- to run the executable

.\joy2key\Debug\joy2key.exe


- to remove the build directory go to the project directory

rmdir /S /Q build
