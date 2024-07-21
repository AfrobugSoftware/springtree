#make file for the base library
project(base LANGUAGES CXX VERSION 0.0.1)


file(GLOB incs ${PROJECT_SOURCE_DIR}/include/*.h ${PROJECT_SOURCE_DIR}/include/*.hpp ${PROJECT_SOURCE_DIR}/*/*.hpp)
file(GLOB srcs ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/*/*.cpp ${PROJECT_SOURCE_DIR}/detail/impl/router.cpp)

SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_subdirectory(${PROJECT_SOURCE_DIR}/bcrypt)

add_library(base SHARED ${incs} ${srcs})
set_target_properties(base PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
	set(BASE_LIB_DIR ${CMAKE_BINARY_DIR}/base/Debug)
else()
	set(BASE_LIB_DIR ${CMAKE_BINARY_DIR}/base/Release)
endif()

if(CMAKE_SYSTEM MATCHES Linux)
	target_compile_options(base PUBLIC -fpremissive -fcoroutines)
elseif(CMAKE_SYSTEM MATCHES Windows)
	target_compile_options(base PUBLIC /bigobj /Zc:__cplusplus /JMC-)
endif()

target_link_directories(base PUBLIC ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
message(STATUS ${BASE_LIB_DIR})
target_include_directories(base PRIVATE ${PROJECT_SOURCE_DIR}/include ${date_INCLUDE_DIR} ${spdlog_INCLUDE_DIR} ${nlohmann_json_INCLUDE_DIR} ${SQLite_INCLUDE_DIR} ${asio_INCLUDE_DIR})
target_link_libraries(base PRIVATE bbcrypt boost::boost spdlog::spdlog fmt::fmt nlohmann_json::nlohmann_json SQLite::SQLite3 LZ4::lz4_static date::date openssl::openssl ZLIB::ZLIB)
target_link_libraries(base PUBLIC bbcrypt)