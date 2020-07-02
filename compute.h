/*
 * compute.h
 * 
 * William Miller
 * Aug 2, 2019
 *
 * Basic computation functions for image enhancement library
 *
 */

#pragma once

#include "enhance.h"
 
double median(std::vector<double> input);

std::vector<double> linspace(const std::pair<double, double> &lim, const size_t &n);
std::vector<std::pair<double, double>> rotate(const std::vector<std::pair<double, double>> &v, const double &rot);
