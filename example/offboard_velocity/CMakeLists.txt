cmake_minimum_required(VERSION 2.8.12)

project(offboard)


if(NOT MSVC)
    #add_definitions("-std=c++11 -Wall -Wextra -Werror")
    add_definitions("-std=c++11")
    # Line below required if /usr/local/include is not in your default includes
    #include_directories(/usr/local/include)
    # Line below required if /usr/local/lib is not in your default linker path
    #link_directories(/usr/local/lib)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

    set(YDLIDAR_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
    list(APPEND YDLIDAR_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/libydlidar_driver.so)
    include_directories(
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/../../
	${CMAKE_CURRENT_BINARY_DIR}
	${YDLIDAR_INCLUDE_DIRS}
    )
else()
    add_definitions("-std=c++11 -WX -W2")
    add_definitions("-D_USE_MATH_DEFINES") # For M_PI
    include_directories(${CMAKE_SOURCE_DIR}/../../install/include)
    link_directories(${CMAKE_SOURCE_DIR}/../../install/lib)
endif()

add_executable(offboard
    offboard_velocity.cpp
)

target_link_libraries(offboard
    dronecode_sdk
    dronecode_sdk_action
    dronecode_sdk_offboard
    dronecode_sdk_telemetry
    ${YDLIDAR_LIBRARIES}
)
