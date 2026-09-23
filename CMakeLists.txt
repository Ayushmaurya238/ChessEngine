cmake_minimum_required(VERSION 3.28)
project(ChessEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)

# Fetch SFML so the project does not depend on a pre-built SFML binary matching
# the exact MinGW/GCC toolchain installed on the user's machine.
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.1.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SFML)

set(ENGINE_SOURCES
    src/Board.cpp
    src/Move.cpp
    src/MoveGenerator.cpp
    src/Evaluator.cpp
    src/SearchEngine.cpp
    src/FENParser.cpp
    src/ZobristHasher.cpp
    src/TranspositionTable.cpp
    src/ChessEngine.cpp
    src/UCI.cpp
)

add_executable(chess_engine src/main.cpp ${ENGINE_SOURCES})
target_include_directories(chess_engine PRIVATE include)
target_compile_features(chess_engine PRIVATE cxx_std_17)

add_executable(chess_gui src/gui_main.cpp ${ENGINE_SOURCES})
target_include_directories(chess_gui PRIVATE include)
target_compile_features(chess_gui PRIVATE cxx_std_17)
target_link_libraries(chess_gui PRIVATE SFML::Graphics)

# Keep the font beside the GUI executable after a build.
add_custom_command(TARGET chess_gui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:chess_gui>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:chess_gui>/assets"
)
