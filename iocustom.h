#ifndef IOCUSTOM_H
#define IOCUSTOM_H

#include <colors.h>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <string>

#include "enhance.h"

const std::string backspace({8, 8, 8, 8});

void print_percent(int current, int total);
std::string datetime();
std::string type2str(int type);

#endif
