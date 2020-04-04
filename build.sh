cd lib
cd cudd
autoreconf -i
./configure
make
cd ..
cd buddy
autoreconf -i
./configure
make
cd ..
cd cacbdd
make
cd ..
cd ..
mkdir build
cd build
cmake ..
make
cd ..
