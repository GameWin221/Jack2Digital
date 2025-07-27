mkdir build

cd build

cmake -G "Ninja" ..
ninja -j4
copy %CD%\Jack2Digital.uf2 E:\