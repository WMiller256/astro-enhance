/*
 * chunk.h
 * 
 * William Miller
 * Jul 2, 2020
 *
 * Minimal class for holding several values
 * associated with an image chunk including
 * the mean and standard deviation, size, 
 * and center point.
 */

#pragma once

#include <cmath>
#include <iomanip>
#include <iostream>

// Struct to store position information
struct Pos {
	long x, y;
};
Pos operator*(const Pos &p, const double &s);
Pos operator*(const double &s, const Pos &p);
Pos operator/(const Pos &p, const double &s);
Pos operator/(const double &s, const Pos &p);
Pos operator+(const Pos &l, const Pos &r);

bool operator==(const Pos &l, const Pos &r);

// Struct to store vertical and horizontal extent
struct Extent {
	long l, r, b, t;
};

class Chunk;
class Chunk {
public:
	Chunk();

	void reset();

	size_t n = 0;
	Pos pos = {0, 0};
	double mean = 0;
	double std = 0;
	double var = 0;

	Chunk operator+(const Chunk& r) const;
	Chunk& operator+=(const Chunk& r);
};
std::ostream& operator<<(std::ostream &os, const Chunk &c);
