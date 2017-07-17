#include <uv.h>

int main() {
    uv_loop_t loop;
    uv_loop_init(&loop);
    uv_run(&loop, UV_RUN_DEFAULT);
}
