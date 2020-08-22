cmake_minimum_required(VERSION 3.1)
project(rtklib_example)


# include_directories(/usr/local/include/rtklib)

find_package(Eigen3)
include_directories(${EIGEN3_INCLUDE_DIR})


add_executable(rinex_example rinex_example.cpp)
target_link_libraries(rinex_example librtklib.a)

add_executable(satpos_example satpos_example.cpp)
target_link_libraries(satpos_example librtklib.a)

add_executable(spp_example spp_example.cpp)
target_link_libraries(spp_example librtklib.a)

add_executable(rtk_example rtk_example.cpp)
target_link_libraries(rtk_example librtklib.a)


add_executable(ppp_satpos_example ppp_satpos_example.cpp)
target_link_libraries(ppp_satpos_example librtklib.a)


add_executable(ppp_example ppp_example.cpp)
target_link_libraries(ppp_example librtklib.a)


