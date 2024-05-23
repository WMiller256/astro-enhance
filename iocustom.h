
#pragma once

#include <ctime>
#include <iostream>
#include <iomanip>
#include <string>

#include "colors.h"
#include "enhance.h"

const std::string backspace({8, 8, 8, 8});

void print_percent(int current, int total);
std::string datetime();
std::string type2str(int type);
