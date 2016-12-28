cmake_minimum_required(VERSION 2.8)
project(qplot)

# Call cmake with my C++1z clang
# CXX="/Users/tom/3rdparty/tools/llvm-svn/build/bin/clang" cmake -DCMAKE_BUILD_TYPE=Debug ..

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lstdc++ -std=c++1z -stdlib=libc++ -nostdinc++ \
          -I/Users/tom/3rdparty/tools/llvm-svn/build/include/c++/v1 \
          -L/Users/tom/3rdparty/tools/llvm-svn/build/lib \
          -Wl,-rpath,/Users/tom/3rdparty/tools/llvm-svn/build/lib")

# -lstdc++

include_directories(..)

set(qplot_src "main.cpp" "test.cpp" "ex1_data1d.cpp" "ex2_gnuplot_array2d.cpp" "ex3_gnuplot_boxplot.cpp" "ex4_python.cpp")

# Executable
add_executable(qplot ${qplot_src})

#clang++ -std=c++11 -stdlib=libc++ -nostdinc++ \
#          -I<libcxx-install-prefix>/include/c++/v1 \
#          -L<libcxx-install-prefix>/lib \
#          -Wl,-rpath,<libcxx-install-prefix>/lib \
#          test.cpp
