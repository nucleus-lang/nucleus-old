cmake_minimum_required (VERSION 3.8)

project(Nucleus CXX)
set(CMAKE_CXX_STANDARD 14)

include_directories(${CMAKE_SOURCE_DIR})

SET (SRCS
	main.cpp
	Language/Lexer.cpp
	Language/Parser.cpp
)

add_executable(Nucleus ${SRCS})