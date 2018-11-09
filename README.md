# StealthDrone-Purdue-
Stealth Drone Project @Purdue University 2018 Fall 

This is Combined SDK from Dronecode SDK and YDLidar SDK.

Dronecode SDK:
https://github.com/Dronecode/DronecodeSDK
YDLidar SDK:
http://ydlidar.com/download

# Instruction
```
$ cmake .
$ make
```
To execute a example.
```
$ cd example
$ cd StealthDrone-Purdue-
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

- YDLidar X4
X4 has 720 Elements that is vector<float>

indexes list
630~89: North
90~269: West
270~449: South
450~629: East
