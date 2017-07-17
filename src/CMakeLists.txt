add_executable(main main.cpp)
target_link_libraries(main PRIVATE Threads::Threads ${LIBUV_STATIC_LIBRARY})
