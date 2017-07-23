# When CMake and libuv fall in love

## Introduction

While I was working to [`uvw`](https://github.com/skypjack/uvw), I found that a lot of people had difficulties to integrate [`libuv`](https://github.com/libuv/libuv) in their `cmake` based projects.  
With this project I tried to get out of `uvw` the stuff used to integrate `libuv`. This isn't probably the best approach and it differs from the ones you can find all around the web. However it's a working solution that is worth mentioning.

## Step by step

The projects is a self contained one from which you can start your own project.  
Below I'm trying to sum up all the steps required to do something similar in an already existent project:

* Create a path `cmake/in` in your root directory and place there a [libuv.in](https://raw.githubusercontent.com/skypjack/libuv_cmake/master/cmake/in/libuv.in) file with the following content:
    ```cmake
    project(libuv-dep NONE)
    cmake_minimum_required(VERSION 3.4)

    include(ExternalProject)

    if(WIN32)
        ExternalProject_Add(
            libuv
            GIT_REPOSITORY https://github.com/libuv/libuv.git
            GIT_TAG v1.x
            DOWNLOAD_DIR ${LIBUV_DEPS_DIR}
            TMP_DIR ${LIBUV_DEPS_DIR}/tmp
            STAMP_DIR ${LIBUV_DEPS_DIR}/stamp
            SOURCE_DIR ${LIBUV_DEPS_DIR}/src
            BUILD_IN_SOURCE 1
            CONFIGURE_COMMAND <SOURCE_DIR>/vcbuild.bat release x86 shared
            BUILD_COMMAND ""
            INSTALL_COMMAND ""
            TEST_COMMAND ""
        )
    else(WIN32)
        ExternalProject_Add(
            libuv
            GIT_REPOSITORY https://github.com/libuv/libuv.git
            GIT_TAG v1.x
            DOWNLOAD_DIR ${LIBUV_DEPS_DIR}
            TMP_DIR ${LIBUV_DEPS_DIR}/tmp
            STAMP_DIR ${LIBUV_DEPS_DIR}/stamp
            SOURCE_DIR ${LIBUV_DEPS_DIR}/src
            BUILD_IN_SOURCE 1
            CONFIGURE_COMMAND sh <SOURCE_DIR>/autogen.sh && ./configure
            BUILD_COMMAND make -j4
            INSTALL_COMMAND ""
            TEST_COMMAND ""
        )
    endif(WIN32)
    ```
    Of course, set the `GIT_TAG` property to your preferred branch.  
    This file will capture the `LIBUV_DEPS_DIR` variable when `configure` is invoked by the parent `CMakeLists.txt`, then it will use it as its root directory. `WIN32` is set automatically by `cmake` and it helps us to invoke the right tools on the right platforms.

* In the main `CMakeLists.txt`, find the required tools to make `libuv` up and running once linked to your targets:
    ```cmake
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    ```
    Then copy the `libuv.in` file over to the right location (I used a directory named `deps` under the project's root directory), run `cmake`, set up include directories and export the libraries you have just compiled:
    ```cmake
    set(LIBUV_DEPS_DIR ${CMAKE_SOURCE_DIR}/deps/libuv)
    configure_file(cmake/in/libuv.in ${LIBUV_DEPS_DIR}/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY ${LIBUV_DEPS_DIR})
    execute_process(COMMAND ${CMAKE_COMMAND} --build . WORKING_DIRECTORY ${LIBUV_DEPS_DIR})
    include_directories(${LIBUV_DEPS_DIR}/src/include)
    find_library(LIBUV_STATIC_LIBRARY NAMES libuv.a libuv PATHS ${LIBUV_DEPS_DIR}/src PATH_SUFFIXES .libs Release NO_DEFAULT_PATH)
    find_library(LIBUV_SHARED_LIBRARY NAMES uv libuv PATHS ${LIBUV_DEPS_DIR}/src PATH_SUFFIXES .libs Release NO_DEFAULT_PATH)
    ```
    See the [`CMakeLists.txt`](https://raw.githubusercontent.com/skypjack/libuv_cmake/master/CMakeLists.txt) file in the project's root directory for furter details.

    The basic idea is that you won't have anymore to include any directory to be able to use `libuv`'s headers. Just put the following include directive in your source file and it will pull in whatever you want: `#include <uv.h>`.  
    On the other side, you can link the static library by means of `target_link_libraries` as:
    ```cmake
    target_link_libraries(my_target PRIVATE Threads::Threads ${LIBUV_STATIC_LIBRARY})
    ```
    Something similar applies to the shared library (both are available once you compiled libuv):
    ```cmake
    target_link_libraries(my_target PRIVATE Threads::Threads ${LIBUV_SHARED_LIBRARY})
    ```
    See the [`CMakeLists.txt`](https://raw.githubusercontent.com/skypjack/libuv_cmake/master/src/CMakeLists.txt) file and [`main.cpp`](https://raw.githubusercontent.com/skypjack/libuv_cmake/master/src/main.cpp) in the source directory for furter details.

## Pros and cons

This method has some pros and some cons, as any other method.  
First of all, if you do that for more than one project, you can put all of them out of your build directory (that I use to wipe out sometimes). This way you can freely clean up the build directory and you won't recompile all your dependencies each time. In the example code, I decided to download and compile dependencies in their owns directories within the `deps` directory (that is, `libuv` is completely contained within `deps/libuv`), so that I can drop it or any of the projects' specific directories if I want to recompile all the dependencies or only one of them.  
Another pros (or cons, it mostly depends on your point of view) is that dependencies are checked only when you run `cmake`. It doesn't matter if they are to be updated or recompiled for some reasons, it won't happen unless you re-run `cmake`. Sometimes the process of checking a dependency is quite annoying and time consuming, so this is a plus for me.  
If something more will come up to my mind, I will add it to this section in future.

## Contributors

If you want to contribute, please send patches as pull requests against the branch master.

## License

Code and documentation Copyright (c) 2017 Michele Caini.  
Code released under [the MIT license](https://github.com/skypjack/libuv_cmake/blob/master/LICENSE).
