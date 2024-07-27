cmake_minimum_required(VERSION 3.20)
project(springtree VERSION 0.1 LANGUAGES CXX)

#setup complier
set(CMAKE_CXX_STANDARD  20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})
set(wxBUILD_SHARED ON)
set(wxUSE_GUI 1)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/wxWidgets-3.2.2.1)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/OpenXLSX)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/wxFreeChart)

add_definitions(-DWXUSINGDLL=1) 

set(CMAKE_UNITY_BUILD TRUE)

find_package(Boost REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(lz4 REQUIRED)
find_package(date REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

file(GLOB incs ${PROJECT_SOURCE_DIR}/include/*.hpp ${PROJECT_SOURCE_DIR}/include/*.h)
file(GLOB srcs ${PROJECT_SOURCE_DIR}/src/*.cpp)

if(CMAKE_SYSTEM MATCHES Windows)
    set(APP WIN32)
    add_compile_options(/bigobj)
    add_compile_options(/Zc:__cplusplus /JMC-)
elseif()
    set(APP "")
endif()

#windows application
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/base)

set(wxWidgets_include_dir ${wxWidgets_root_dir}/include)
set(wxWidgets_libraries_dir ${wxWidgets_root_dir}/lib/vc_x64_dll)
if(CMAKE_SYSTEM MATCHES Windows)
	set(wxWidgets_setup_dir ${wxWidgets_libraries_dir}/mswu)
endif()

add_executable(pharmaoffice ${APP} ${incs} $<$<BOOL:${WIN32}>:pharmaoffice.rc> ${srcs} ${PROJECT_SOURCE_DIR}/main.manifest)

set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB)
##target_precompile_headers(pharmaoffice PUBLIC ${incs})
target_link_directories(pharmaoffice PUBLIC ${PROJECT_SOURCE_DIR}/thirdparty/wxWidgets-3.2.2.1/lib/vc_x64_dll)
target_include_directories(pharmaoffice PUBLIC ${PROJECT_SOURCE_DIR}/base/include ${PROJECT_SOURCE_DIR}/include ${spdlog_INCLUDE_DIR} ${nlohmann_json_INCLUDE_DIR} ${SQLite_INCLUDE_DIR} ${date_INCLUDE_DIR} ${asio_INCLUDE_DIR})
target_link_libraries(pharmaoffice PUBLIC boost::boost spdlog::spdlog fmt::fmt nlohmann_json::nlohmann_json SQLite::SQLite3 LZ4::lz4_static date::date openssl::openssl ZLIB::ZLIB)
target_link_libraries(pharmaoffice PUBLIC wxFreeCharts base OpenXLSX wx::aui wx::html wx::propgrid wx::qa wx::net wx::core wx::base)