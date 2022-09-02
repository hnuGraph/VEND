cmake_minimum_required(VERSION 2.8.7)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++17 -L/usr/local/lib -L/usr/local/lib64")
project(Vend CXX)

# probuf


set(VEND_SRC_INCLUDE ${PROJECT_SOURCE_DIR}/src/include ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/src/include/thirdpart)
add_subdirectory(./src/include/thirdpart/googletest)
set(GTEST
        gtest
        pthread)

# db library
file(GLOB DB_ENGINE_SRC ${PROJECT_SOURCE_DIR}/src/dbengine/*.cpp ${PROJECT_SOURCE_DIR}/src/include/dbengine/*.h)
add_library(DB_ENGINE STATIC ${DB_ENGINE_SRC} )
target_link_libraries(DB_ENGINE librocksdb.a -lpthread -lsnappy -lbz2 -lzstd -ldl libz.so -llz4 )





include_directories(${VEND_SRC_INCLUDE})
file(GLOB_RECURSE ALL_SRCS "*.h" "*.cpp" ".cc")


#FIND_PACKAGE(OpenMP REQUIRED)

#openmp
if (OPENMP_FOUND)
    message("OPENMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif ()

#link_directories(${PROJECT_SOURCE_DIR}/include/lib ${PLATFORM_LIBS})

add_library(SRCS STATIC ${ALL_SRCS}  )
target_link_libraries(SRCS  )

option(TEST_T "options for triangle" OFF)
option(TEST "options for intersection filter " OFF)
option(TEST_T1 "count triangles by vend" OFF)
option(TEST_T2 "vend filter adj " OFF)

if (TEST)
    add_definitions(-DVEND_LEVEL=-1)
endif (TEST)

if (TEST_T)
    add_definitions(-DVEND_LEVEL=0)
endif (TEST_T)


if (TEST_T1)
    add_definitions(-DVEND_LEVEL=1)
endif (TEST_T1)
if (TEST_T2)
    add_definitions(-DVEND_LEVEL=2)
endif (TEST_T2)
add_executable(Vend main.cpp ${PROTO_SRC_DIR})
add_executable(Triangle triangle_test.cpp)


add_executable(DiskTriangle src/triangle/diskTriangleCount.cpp)
target_link_libraries(DiskTriangle SRCS DB_ENGINE)

target_link_libraries(Vend SRCS DB_ENGINE)
target_link_libraries(Vend ${GTEST})

target_link_libraries(Triangle SRCS DB_ENGINE)
add_executable(TEST test.cpp)
target_link_libraries(TEST SRCS DB_ENGINE)


SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

