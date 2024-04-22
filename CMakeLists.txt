cmake_minimum_required(VERSION 2.8.7)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++17  -msse4.1 -O3")
project(Vend CXX)


FIND_PACKAGE(OpenMP REQUIRED)

set(VEND_SRC_INCLUDE ${PROJECT_SOURCE_DIR}/src/include ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/src/include/thirdpart)
include_directories(${VEND_SRC_INCLUDE})

enable_testing()
add_subdirectory(./src/include/thirdpart/googletest)
include_directories(src/include/thirdpart/googletest/include/)
set(GTEST
        gtest
        pthread)

# db library
file(GLOB DB_ENGINE_SRC ${PROJECT_SOURCE_DIR}/src/dbengine/*.cpp ${PROJECT_SOURCE_DIR}/src/include/dbengine/*.h)
add_library(DB_ENGINE STATIC ${DB_ENGINE_SRC} )
target_link_libraries(DB_ENGINE librocksdb.a -lpthread -lsnappy -lbz2 -lzstd -ldl libz.so -llz4  -lssl
        -lcrypto -lm  -lcurl)



file(GLOB_RECURSE ALL_SRCS "*.h" "*.cpp")


#openmp
if (OPENMP_FOUND)
    message("OPENMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif ()

add_library(SRCS STATIC ${ALL_SRCS})



add_executable(Vend main.cpp ${PROTO_SRC_DIR})
target_link_libraries(Vend SRCS DB_ENGINE)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS}  -Wall -g2 -ggdb -w -pg ")

