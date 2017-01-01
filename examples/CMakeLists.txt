cmake_minimum_required(VERSION 3.0)
project(qplot_examples)


# Options
option (USE_BOOST_VARIANT "Use boost::variant instead of C++17 std::variant" ON)

# C++1z tested with Clang 4.0 only, sorry. Edit as nessary
set(LLVM_PATH "$ENV{LLVM_ROOT}" CACHE PATH "Path to llvm")


# Executable
set(src "main.cpp"
	    "example_python.cpp"
	    "example_gnuplot.cpp"
	    "example_process.cpp"
	    "test.cpp"
	    )

include_directories(..)
add_executable(qplot_examples ${src})

if (USE_BOOST_VARIANT)
	find_package(Boost REQUIRED)
	if(Boost_FOUND)
		include_directories(${Boost_INCLUDE_DIRS})
	    add_definitions(-DWITH_BOOST_VARIANT)
	endif()
	target_compile_options(qplot_examples PRIVATE -std=c++14)
else()
	# Compile with C++1z
	include_directories("${LLVM_PATH}/include/c++/v1")
	target_compile_options(qplot_examples PRIVATE -nostdinc++ -std=c++1z -stdlib=libc++)
	# Link to C++1z libc++
	target_link_libraries(qplot_examples "-lstdc++")
	target_link_libraries(qplot_examples "-L${LLVM_PATH}/lib")
	target_link_libraries(qplot_examples "-Wl,-rpath,${LLVM_PATH}/lib")
endif()
