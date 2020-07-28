# Unified framework for BDD packages
## Requirement
 - modern C compiler and cmake
 - (optional) python3 and Opentuner https://github.com/jansel/opentuner
 
When you use Opentuner, I recommend to install it by cloning the github repository and running "python3 setup.py install" instead of "pip3 install".
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

## Implement your application

## Implement your BDD package
