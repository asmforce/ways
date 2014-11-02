#include "ways.hpp"

#include <iostream>
#include <cstdlib>

#include <elib/aliases.hpp>
using namespace elib::aliases;


int main( int argc, char **argv ) {
    if (Ways::translate(std::cin, std::cout)) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
