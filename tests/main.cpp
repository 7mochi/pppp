#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

#include <iostream>

int main(int argc, char** argv) {
    std::cout << std::unitbuf;

    doctest::Context context(argc, argv);
    return context.run();
}
