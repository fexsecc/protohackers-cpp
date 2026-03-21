cmake_minimum_required(VERSION 4.1)

if(NOT CMAKE_C_COMPILER)
  set(CMAKE_C_COMPILER "clang")
endif()

if(NOT CMAKE_CXX_COMPILER)
  set(CMAKE_CXX_COMPILER "clang++")
endif()

project(MeansToAnEnd VERSION 0.1.0 LANGUAGES CXX)

# Compiler settings
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to Release if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Source layout
file(GLOB_RECURSE CPP_SRC CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE CPPM_SRC CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cppm"
)

add_executable(${PROJECT_NAME} ${CPP_SRC})

# Add modules
target_sources(${PROJECT_NAME}
    PRIVATE
        FILE_SET CXX_MODULES FILES
            ${CPPM_SRC}
)
