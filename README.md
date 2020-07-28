# Auto-tuning framework for BDD packages
## Requirement
 - modern C compiler and cmake
 - python3 and Opentuner https://github.com/jansel/opentuner
 
I recommend to install Opentuner by cloning the github repository and running "python3 setup.py install" instead of "pip3 install".
## Get started
### Build
Please clone this repository with option "--recursive" (if you miss it, please run "git submodule update -i" in the project directory).
You can build example applications with option "-DBDD_EXAMPLE=ON" given to cmake.
```
git clone --recursive https://github.com/MyskYko/bdd
cd bdd
mkdir build
cd build
cmake -DBDD_EXAMPLE=ON ..
make
cd ..
```
The executables of several application examples will be generated in the directory "build/example".
Make sure the versions of your C compiler and cmake are new enough.

### Example application (aig2bdd)
One of the example applications is to build BDDs from AIG. Its executable is named "aig2bdd".
This executable will construct BDDs for all outputs of the AIG with BDD nodes shared.
Please prepare a small AIG and run the following.
```
./build/example/aig2bdd -s -v 1 your.aig
```
This (option -v 1) will show simple statistics of BDDs as follows:
```
$ ./build/example/aig2bdd -s -v 1 C17.aig
time : 0 ms
Shared BDD nodes = 11
Sum of BDD nodes = 14
```
(option -s turns off reading name of inputs/outputs. Without this, the program will display warnings for AIGs not supporting names.)

### BDD packages
This framework supports the following BDD packages. () shows the library name used in cmake.
 - CUDD (cudd)
 - BuDDy (buddy)
 - CacBDD (cacbdd)
 - SimpleBDD (simplebdd)
 - AtBDD (atbdd)

There is a manager header for each package in "include" directory.
The packages are placed as submodule in "lib" directory.

AtBDD is a custom BDD package newly created. It is based on SimpleBDD and integrates dynamic cache-management of CacBDD.

The example application "aig2bdd" has option "-p" to switch packages.

 - CUDD (by default)
```
./build/example/aig2bdd -s -v 1 your.aig
```
 - BuDDy
```
./build/example/aig2bdd -s -v 1 -p 1 your.aig
```
 - CacBDD
 ```
./build/example/aig2bdd -s -v 1 -p 2 your.aig
```
 - SimpleBDD
 ```
./build/example/aig2bdd -s -v 1 -p 3 your.aig
```
 - AtBDD
```
./build/example/aig2bdd -s -v 1 -p 4 your.aig
```

### Auto-tuning
Auto-tuning can be performed through "tuner.py", our interface to Opentuner.
Please prepare a midium size AIG, which "aig2bdd" takes around 10 sec. Then run the follwoing.
```
./tuner.py --test-limit 10 "./build/example/aig2bdd -s your.aig" ./include/CuddMan.hpp
```
"aig2bdd" uses CUDD by default, so we designate as the last argument the header of CUDD manager where the parameters and their ranges are defined.
This command adjusts the parameters of CUDD by testing 10 sets of parameters to minimize the runtime required to build BDDs for your AIG.
Because there is a time limit for each run, 60 sec by default, this tuning takes at most 10 min.
The time limit for each run can be changed by option "--test-timeout" followed by an integer in sec.

After the tuning successfully finished, the best parameters are written in the file "_CuddMan.hpp_setting.txt".
CUDD manager will automatically read this file in the running directory to set the paramters if exists.

Here is an execution trace.
```
$ ./build/example/aig2bdd -s -v 1 dalu.aig
time : 7436 ms
Shared BDD nodes = 3268041
Sum of BDD nodes = 9726501
$ ./tuner.py --test-limit 10 "./build/example/aig2bdd -s dalu.aig" ./include/CuddMan.hpp
['Log', '100', '1000000000']
['Log', '100', '1000000000']
['Log', '100', '1000000000']
['Int', '1', '100']
['Bool']
['Switch', '12']
['Int', '1', '100']
{0: 275, 1: 863377, 2: 33830, 3: 34, 4: True, 5: 2, 6: 35}
{'time': 11.416247844696045, 'returncode': 0, 'stdout': b'', 'timeout': False, 'stderr': b''}
{0: 2925, 1: 276782333, 2: 217867842, 3: 42, 4: False, 5: 0, 6: 95}
{'time': 17.748870134353638, 'returncode': 0, 'stdout': b'', 'timeout': False, 'stderr': b''}
{0: 64635, 1: 102, 2: 100, 3: 48, 4: True, 5: 4, 6: 99}
{'time': 8.551174402236938, 'returncode': 0, 'stdout': b'', 'timeout': False, 'stderr': b''}
{0: 1328495, 1: 5979237, 2: 4323, 3: 59, 4: False, 5: 0, 6: 30}
{'time': 8.728569746017456, 'returncode': 0, 'stdout': b'', 'timeout': False, 'stderr': b''}
[    47s]    INFO opentuner.search.plugin.DisplayPlugin: tests=4, best {0: 64635, 1: 102, 2: 100, 3: 48, 4: True, 5: 4, 6: 99}, cost time=8.5512, found by UniformGreedyMutation
{0: 64635, 1: 102, 2: 100, 3: 49, 4: True, 5: 4, 6: 95}
{'time': 8.528369665145874, 'returncode': 0, 'stdout': b'', 'timeout': False, 'stderr': b''}
{0: 64635, 1: 111, 2: 100, 3: 48, 4: False, 5: 4, 6: 99}
{'time': 7.213785409927368, 'returncode': 0, 'stdout': b'', 'timeout': False, 'stderr': b''}
{0: 450912, 1: 100, 2: 100, 3: 46, 4: True, 5: 2, 6: 96}
{'time': 5.719759702682495, 'returncode': 0, 'stdout': b'', 'timeout': False, 'stderr': b''}
{0: 3208, 1: 100, 2: 100, 3: 34, 4: True, 5: 6, 6: 97}
{'time': 10.565606832504272, 'returncode': 0, 'stdout': b'', 'timeout': False, 'stderr': b''}
[    79s]    INFO opentuner.search.plugin.DisplayPlugin: tests=8, best {0: 450912, 1: 100, 2: 100, 3: 46, 4: True, 5: 2, 6: 96}, cost time=5.7198, found by NormalGreedyMutation
{0: 450912, 1: 100, 2: 100, 3: 46, 4: True, 5: 0, 6: 96}
{'time': 5.713878631591797, 'returncode': 0, 'stdout': b'', 'timeout': False, 'stderr': b''}
{0: 54266371, 1: 100, 2: 100, 3: 46, 4: True, 5: 2, 6: 96}
{'time': inf, 'returncode': -9, 'stdout': b'', 'timeout': True, 'stderr': b''}
{0: 450912, 1: 170622, 2: 100, 3: 46, 4: True, 5: 2, 6: 96}
{'time': 7.168837785720825, 'returncode': 0, 'stdout': b'', 'timeout': False, 'stderr': b''}
{0: 450912, 1: 100, 2: 100, 3: 39, 4: True, 5: 2, 6: 1}
{'time': 5.840299606323242, 'returncode': 0, 'stdout': b'', 'timeout': False, 'stderr': b''}
[   159s]    INFO opentuner.search.plugin.DisplayPlugin: tests=12, best {0: 450912, 1: 100, 2: 100, 3: 46, 4: True, 5: 0, 6: 96}, cost time=5.7139, found by UniformGreedyMutation
[   159s]    INFO opentuner.search.plugin.DisplayPlugin: tests=12, best {0: 450912, 1: 100, 2: 100, 3: 46, 4: True, 5: 0, 6: 96}, cost time=5.7139, found by UniformGreedyMutation
### Results ###
100 < 450912 < 1000000000
100 < 100 < 1000000000
100 < 100 < 1000000000
1 < 46 < 100
True
0
1 < 96 < 100
$ cat _CuddMan.hpp_setting.txt
450912
100
100
46
True
0
96
$ ./build/example/aig2bdd -s -v 1 dalu.aig
time : 4276 ms
Shared BDD nodes = 3268041
Sum of BDD nodes = 9726501
```
The runtime was reduced by 3.1 sec (7.4 sec -> 4.3 sec).

Please try other packages. For example, BuDDy can be tuned by the following command.
```
./tuner.py --test-limit 10 "./build/example/aig2bdd -s -p 1 your.aig" ./include/BuddyMan.hpp
```

## Implement your application
You can implement your application program just in "example" directory or import this project as a library of your project.
The former choice would be easier in that you only need to write one C++ code without touching any other files, but I recommend the latter for management and extendability purpose.

For example, we create a new project in a directory named "testproject" and clone this repository inside.
```
mkdir testproject
cd testproject
git clone --recursive https://github.com/MyskYko/bdd
```
If you use git, you can import this repository by "git submodule add" instead of "git clone".

We create "main.cpp" as shown in below.
This realizes a very simple application, which checks XOR of *a* and *b* is the same as Or(And(*a*, Not(*b*)), And(Not(*a*), *b*)).
```
#include <iostream>
#include <CuddMan.hpp>

template <typename node>
void check_xor(Bdd::BddMan<node> & man) {
  if(man.GetNumVar() < 2) abort();
  node a = man.IthVar(0);
  node b = man.IthVar(1);
  node x = man.Xor(a, b);
  node y = man.Or(man.And(a, man.Not(b)), man.And(man.Not(a), b));
  if(x != y) std::cout << "wrong" << std::endl;
  else std::cout << "Correct" << std::endl;
}

int main() {
  int nVars = 2;
  int nVerbose = 0;
  Bdd::CuddMan man(nVars, nVerbose);
  check_xor(man);
  return 0;
}
```
In the main function, it instanciates CUDD manager included from "CuddMan.hpp" and pass it to the function "check_xor".
The number of variables and the verbosing level are required to initialize the manager. Note currently there is no function to increase the number of variables.
The function "check_xor" recieves the manager in the form of super class "BddMan<node>" as reference, where the type "node" representing BDD node is specified as a template parameter.
This function can be shared by different BDD packages.
The member functions of BddMan are listed in "include/BddMan.hpp".

To build this project, we use cmake. It links libraries and sets include directories.
Create "CMakefile.txt" as follows:
```
cmake_minimum_required(VERSION 3.8)
project(testproject CXX C)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-g -O3)

add_subdirectory(bdd)

add_executable(test main.cpp)
target_link_libraries(test bdd cudd)
```
The former 5 lines are for cmake configuration. Our BDD framework is imported by "add_subdirectory".
We specify "main.cpp" to be compiled into executable "test", which depends on libraries "bdd" and "cudd".
The library "cudd" is just a plain CUDD library. The library "bdd" provides our interface franmework.
When you use a package other than CUDD, you have to modify this "cudd" to the one you use.

Finally, we can compile and run the program as follows:
```
mkdir build
cd build
cmake ..
make
./test
```
Make sure you see "Correct" in your screen.

## Integrate your BDD package
Your BDD package can be integrated into our framework by implementing a manager class which inherits the class "BddMan" in "include/BddMan.hpp".
Then, application implemented before will run on your BDD package without modification.

### Mandatory functions
Here we list up the mandatory functions.

```
int GetNumVar();
```
This returns the number of variables allocated.

```
node Const0();
node Const1();
node IthVar( int i );
```
These are functions to return fundamental nodes. "IthVar" is not subject to variable ordering.

```
int Var( node const & x );
node Then( node const & x );
node Else( node const & x );
uint64_t Id( node const & x );
```
These functions return properties of non-constant nodes.
"Id" enables users to use std::map containters. In my view, BDD nodes are likely to have ID inside a BDD package, so this should be easily implemented.

```
node Not( node const & x );
node And( node const & x, node const & y );
```
These two operations must be implemented. The other operations are realized using them unless defined, which may not be efficient.
As the implementation matures, it is encouraged to overwrite other operations to improve the performance.

### Semi-madatory functions
Depending on your implementation, the following functions are required to be defined.

```
node Regular( node const & x ) { return x; }
bool IsCompl( node const & x ) { (void)x; return 0; }
```
If a package adopts negated edges, these two functions must be overwritten. Otherwise, these predefined functions make your package compatible.
Negated edges are implemented as an extra attribute of "node" to express wether the node is complemented(negated) or not.
"Regular" returns a non-complimented node. If a node is complimented, the negation of the node will be returned. Otherwise, the node itself will be returned.
"IsCompl" just returns the attribute of the node.

```
Level( int i ) { return i; }
void Reorder() { std::cerr << "Reorder is not implemented" << std::endl; }
void Dvr() { std::cerr << "Dvr is not implemented" << std::endl; }
void DvrOff() {}
```
If a package is capable of reordering, these function should be implemented. "Level" returns the current position of the variable, where smaller is prior.
"Dvr" is an abbreviation of dynamic variable reordering, which performs reordering during BDD operation under some criteria.

### Reference count
For garbage collection and reordering, a package has to maintain(count) external reference to nodes.
In this framework, this reference task is not imposed on the users. So, a package is required to do it internally.
As you can see in the manager header of SimpleBDD "include/SimpleBdd.hpp", this can be implemented with a wrapper class of node.
While a node is actually an index, we wrap the index by a class which registers the node with reference manager in constructor and deregisters it in destructor.

### Auto-tuning
To apply auto-tuning to your BDD package, you need to write the list of parameters in the header of the manager class.

Between a line "// Param" and a line "// end", the type and range of each paramter is written per line.
"Int" and "Log" are integer types, but "Log" has a different scaling manner in tuning.
"Pow" is quiotient of power of 2 and has a similer scaling manner as "Log".
"Switch" is an integer type and ranges from 0 to the specified value but is not assumed to have gradient characteristic.
"Bool" is a boolean type (True or False).
```
// Param
// Int 1 100
// Int -25 30
// Log 1000 1000000000
// Pow 10 31
// Switch 5
// Bool
// end
```

"tuner.py" will find this sequence of parameters and write the values to be tried in a file named "_yourheader.hpp_setting.txt", assuming a name of your header is "yourheader.hpp".
Your manager is sought to read the file and set the paramaters accordingly, maybe in constructor.
The values are ordered in the same order as the sequence.
The following is an example of the assignment for the parameter list above.
```
38
-3
102143
1048576
5
False
```


## Afterword
If you have a question/suggestion, please send e-mail to miyasaka at cad.t.u-tokyo.ac.jp
