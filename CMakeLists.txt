cmake_minimum_required(VERSION 3.16)
project(bminor_frontend LANGUAGES CXX)

add_library(bminor_lexer src/lexer.cpp)
target_include_directories(bminor_lexer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(bminor_lexer PUBLIC cxx_std_17)

add_library(bminor_parser src/parser.cpp)
target_link_libraries(bminor_parser PUBLIC bminor_lexer)

add_library(bminor_highlighter src/highlighter.cpp src/render.cpp)
target_link_libraries(bminor_highlighter PUBLIC bminor_parser)

if(MSVC)
    target_compile_options(bminor_lexer PRIVATE /W4)
    target_compile_options(bminor_parser PRIVATE /W4)
    target_compile_options(bminor_highlighter PRIVATE /W4)
else()
    target_compile_options(bminor_lexer PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(bminor_parser PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(bminor_highlighter PRIVATE -Wall -Wextra -Wpedantic)
endif()
