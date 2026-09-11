cmake_minimum_required(VERSION 3.22)

project(
	priority_queue
    VERSION 1.0.0
    LANGUAGES C)

# cmake-format: off
set(CMAKE_C_STANDARD          17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS        OFF)

# Define executable name
set(EXECUTABLE_NAME  queue)

add_executable(
	${EXECUTABLE_NAME}
	queue.c
	tests/queue_p_tests.c
)

target_include_directories(
	${EXECUTABLE_NAME}
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
)

target_compile_options(
	${EXECUTABLE_NAME}
	PRIVATE
		-Wall
		-Wextra
		-Wpedantic
)

target_compile_definitions(
	${EXECUTABLE_NAME}
	PRIVATE
		_POSIX_C_SOURCE=200809L
)

target_link_libraries(
	${EXECUTABLE_NAME}
	PRIVATE
		cunit
)
