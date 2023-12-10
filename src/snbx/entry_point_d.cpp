#include "app.hpp"

int main() {
    app_init();
    while (app_update()) {}
    app_destroy();
    return 0;
}