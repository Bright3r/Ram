cmake_minimum_required(VERSION 3.20)

set (CMAKE_CXX_STANDARD 20)

project(62)

add_executable(main
	src/main.cpp
	src/EdgeColoredUndirectedGraph.cpp
	src/GraphUtils.cpp
	src/Utils.cpp
	src/CanonicalAugmentation.cpp
)

target_include_directories(
	main PRIVATE
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/vendor/nauty2_9_1
	${CMAKE_SOURCE_DIR}/vendor/cadical/src
)

target_link_libraries(
	main PRIVATE
	${CMAKE_SOURCE_DIR}/vendor/nauty2_9_1/nauty.a
	${CMAKE_SOURCE_DIR}/vendor/cadical/build/libcadical.a
)
