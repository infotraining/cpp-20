get_filename_component(PROJECT_NAME_STR ${CMAKE_SOURCE_DIR} NAME)
string(REPLACE " " "_" ProjectId ${PROJECT_NAME_STR})

cmake_minimum_required(VERSION 3.8)
project(${PROJECT_NAME_STR})

#----------------------------------------
# Application
#----------------------------------------
aux_source_directory(. SRC_LIST)

# Headers
file(GLOB HEADERS_LIST "*.h" "*.hpp")
add_executable(${PROJECT_NAME} ${SRC_LIST} ${HEADERS_LIST})

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /await  /std:c++latest)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE "-std=c++2a" "-fconcepts-diagnostics-depth=2" "-fmodules-ts")
endif()

#----------------------------------------
# Tests
#----------------------------------------
enable_testing() 
add_test(tests ${PROJECT_NAME})
