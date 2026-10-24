cmake_minimum_required(VERSION 3.25)
project(LithographyControlStudio VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LCS_BUILD_TESTS "Build unit tests" ON)
option(LCS_BUILD_CLI "Build CLI" ON)
option(LCS_BUILD_DESKTOP "Build desktop shell" ON)

add_library(lcs_core
    src/Math.cpp
    src/VirtualMachine.cpp
    src/Configuration.cpp
    src/Types.cpp
    src/Telemetry.cpp
    src/AlarmManager.cpp
    src/SimulationEngine.cpp
    src/Environment.cpp
    src/Units.cpp
    src/StageModel.cpp
    src/WaferModel.cpp
    src/OpticsModel.cpp
    src/JobModel.cpp
    src/SafetyInterlock.cpp
)

target_include_directories(lcs_core PUBLIC include)
target_compile_features(lcs_core PUBLIC cxx_std_23)

if(MSVC)
    target_compile_options(lcs_core PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(lcs_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(LCS_BUILD_CLI)
    add_executable(lcs_cli applications/CLI/main.cpp)
    target_link_libraries(lcs_cli PRIVATE lcs_core)
endif()

if(LCS_BUILD_DESKTOP)
    add_executable(lcs_desktop applications/Desktop/main.cpp)
    target_link_libraries(lcs_desktop PRIVATE lcs_core)
endif()

if(LCS_BUILD_TESTS)
    enable_testing()
    add_executable(lcs_tests tests/test_core.cpp tests/test_simulation_models.cpp)
    target_link_libraries(lcs_tests PRIVATE lcs_core)
    add_test(NAME lcs_core_tests COMMAND lcs_tests)
endif()

install(TARGETS lcs_core ARCHIVE DESTINATION lib LIBRARY DESTINATION lib RUNTIME DESTINATION bin)
if(LCS_BUILD_CLI)
    install(TARGETS lcs_cli RUNTIME DESTINATION bin)
endif()
if(LCS_BUILD_DESKTOP)
    install(TARGETS lcs_desktop RUNTIME DESTINATION bin)
endif()
