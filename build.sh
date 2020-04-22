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
rm -rf build
mkdir build
cd build
cmake -DBDD_TEST=ON ..
make
cd ..
