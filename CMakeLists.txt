cmake_minimum_required(VERSION 3.20)

set (CMAKE_CXX_STANDARD 20)

project(62)

add_executable(main
	src/main.cpp
)

target_include_directories(
	main PRIVATE
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/vendor/nauty2_9_1
)

target_link_libraries(
	main PRIVATE
	${CMAKE_SOURCE_DIR}/vendor/nauty2_9_1/nauty.a
	# ${CMAKE_SOURCE_DIR}/vendor/nauty2_9_1/nautyL1.a
)
