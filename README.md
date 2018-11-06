# StealthDrone-Purdue-
Stealth Drone Project @Purdue University 2018 Fall 


# Instruction
```
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./offboard
```

# NOTICE
- CMakeList.txt
1. Add include directories.
2. Modify definitions.
```
add_definitions("-std=c++11 -Wall -Wextra -Werror")
```
Just remove warning options like this:
```
add_definitions("-std=c++11")
```
3. Settings To use Threads.
write this code under the 'add_definitions'
```
find_package(Threads)
```
And after add_executable write this.
```
target_link_libraries({PRJ NAME} ${CMAKE_THREAD_LIBS_INIT})
```
