#Bcrypt.cpp
project(bbcrypt)

set(CMAKE_CXX_STANDARD 11)

add_library(bbcrypt src/bcrypt.cpp src/blowfish.cpp)
target_include_directories(bbcrypt PRIVATE src/)
target_include_directories(bbcrypt PRIVATE include/)
