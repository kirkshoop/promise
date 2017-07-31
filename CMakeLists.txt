cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

get_filename_component(SAMPLE_PROJECT "${CMAKE_CURRENT_SOURCE_DIR}" NAME)

project(${SAMPLE_PROJECT} LANGUAGES C CXX)

MESSAGE( STATUS "Location: " ${CMAKE_CURRENT_SOURCE_DIR} )

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Export compilation data-base

# Target for fetching packages
add_custom_target(fetch_packages)

include(FindPackageHandleStandardArgs)

include(${CMAKE_CURRENT_SOURCE_DIR}/shared.cmake)

# define the sources
set(SAMPLE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# define the resources
set(SAMPLE_RESOURCES
)

add_executable(${SAMPLE_PROJECT} ${SAMPLE_SOURCES} ${SAMPLE_RESOURCES})

target_compile_options(${SAMPLE_PROJECT} PUBLIC ${SHARED_COMPILE_OPTIONS})
target_compile_features(${SAMPLE_PROJECT} PUBLIC ${SHARED_COMPILE_FEATURES})

target_include_directories(${SAMPLE_PROJECT} SYSTEM PUBLIC 
)

target_link_libraries(${SAMPLE_PROJECT} 
)

# configure unit tests via CTest
enable_testing()
set(CTEST_CONFIGURATION_TYPE "${JOB_BUILD_CONFIGURATION}")

add_test(NAME RunTests
     WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
     COMMAND ${SAMPLE_PROJECT} ${TEST_ARGS})
