cd lib
cd cudd
autoreconf -i
./configure --enable-obj
make clean
make
cd ..
cd buddy
autoreconf -i
./configure
make clean
make
cd ..
cd cacbdd
make cl
make
cd ..
cd ..
mkdir build
cd build
cmake ..
make clean
make
cd ..
