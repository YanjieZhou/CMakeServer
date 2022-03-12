# CMakeList.txt : CMake project for CMakeServer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (CMakeServer "CMakeServer.cpp" "CMakeServer.h" "lock/blockingqueue.h" "Mysql/connection.h" "Mysql/connection.cpp" "log/log.cpp" "log/log.h" "timer/timer.h" "timer/timer.cpp" "threadpool/threadpool.h"  "http/httpconn.h")

# TODO: Add tests and install targets if needed.
