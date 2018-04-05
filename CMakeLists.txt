cmake_minimum_required(VERSION 3.9)
project(sandmp)

find_program(
        CLANG_TIDY_EXE
        NAMES "clang-tidy"
        DOC "Path to clang-tidy executable"
)
if(NOT CLANG_TIDY_EXE)
    message(STATUS "clang-tidy not found.")
else()
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-checks=*,-clang-analyzer-alpha.*")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(WARNINGS "-Wall -pedantic-errors -ansi -Wextra -Winit-self -Wold-style-cast -Werror -Woverloaded-virtual \
    -Wuninitialized -Wmissing-declarations -Winit-self")
set(STANDARDS "-ansi -std=c++17")
set(CMAKE_CXX_FLAGS "-O0 ${WARNINGS} ${STANDARDS}")

include_directories( ./src/)
set(SOURCE_FILES src/main.cpp src/ch2/basics.h src/ch2/basics.cpp src/ch3/lock.cpp src/ch3/lock.h src/ch3/threadsafe_stack.h src/ch3/hierarchical_mutex.h src/ch3/lock_pair.h src/ch3/hierarchical_mutex.cpp src/ch3/readerwriter_mutex.cpp src/ch3/readerwriter_mutex.h src/ch3/lock_pair.cpp src/ch4/sync.h src/ch4/threadsafe_queue.h src/ch4/double_buffer.cpp src/ch4/double_buffer.h src/ch4/sync.cpp src/ch4/threadsafe_queue.cpp src/ch4/quicksort.h src/ch5/spinlock.cpp src/ch5/spinlock.h src/ch4/quicksort.cpp src/ch4/promises.cpp src/ch4/promises.h src/ch6/queue.h src/ch6/concurrent.cpp src/ch6/concurrent.h src/ch6/lookup_table.h src/ch6/list.h)
add_executable(sandmp ${SOURCE_FILES})

set_target_properties(
        sandmp PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        COMPILE_FLAGS "${WARNING_FLAGS}"
)

target_link_libraries(sandmp -lfolly -lglog -larrow -lpthread -lboost_system -lboost_iostreams -lboost_filesystem -lboost_thread)

if(CLANG_TIDY_EXE)
    set_target_properties(
            sandmp PROPERTIES
            CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
    )
endif()