cmake_minimum_required(VERSION 3.6)

##############
# Vcpkg integration - uncomment if necessery
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

if (DEFINED CMAKE_TOOLCHAIN_FILE)
  message(STATUS "Vcpkg integration script found: " ${CMAKE_TOOLCHAIN_FILE})
else()
  message(STATUS "Vcpkg integration script NOT found!")
endif()


##################
# Project
get_filename_component(PROJECT_NAME_DIR ${CMAKE_SOURCE_DIR} NAME)
string(REPLACE " " "_" PROJECT_ID ${PROJECT_NAME_DIR})

set(PROJECT_MAIN ${PROJECT_ID})
project(${PROJECT_MAIN})


####################
# Packages & libs
find_package(Catch2 CONFIG REQUIRED)    

####################
# Sources & headers
aux_source_directory(. SRC_LIST)
file(GLOB HEADERS_LIST "*.h" "*.hpp")

####################
# Main app
add_executable(${PROJECT_MAIN} ${SRC_LIST} ${HEADERS_LIST})

target_link_libraries(${PROJECT_MAIN} PRIVATE ${PROJECT_LIB} Catch2::Catch2)

if (MSVC)
    target_compile_options(${PROJECT_MAIN} PRIVATE /await  /std:c++latest)
else()
    target_compile_features(${PROJECT_MAIN} PUBLIC cxx_std_20)
    #target_compile_options(${PROJECT_MAIN} PRIVATE "-fconcepts-diagnostics-depth=2")
    #target_compile_options(${PROJECT_MAIN} PRIVATE "-fmodules-ts")
endif()