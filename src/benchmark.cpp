#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <filesystem>
#include <memory>
#include <iomanip>
#include <sstream>

#include <common.h>

#include "bench/time_copy.h"
#include "utils/cli_helper.h"

int main(int argc, char* argv[]) {
    display_banner();

    run_benchmark(argc, argv);
    return 0;
}
/// TODO: load page.json