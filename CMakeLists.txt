# requires minimum cmake version
cmake_minimum_required(VERSION 3.5)

# produce the cmake var PROJECT_NAME
project(luvharris)

if(NOT CMAKE_BUILD_TYPE)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "Release")
endif()

# mandatory use of these packages
find_package(YCM REQUIRED)
find_package(OpenCV REQUIRED)
find_package(YARP COMPONENTS os REQUIRED)
find_package(event-driven REQUIRED)

add_executable(${PROJECT_NAME} luvharrisexample.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE  YARP::YARP_OS
                                            YARP::YARP_init
                                            ${OpenCV_LIBRARIES}
                                            ev::event-driven)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

