/*
 * chunk.c++
 *
 * William Miller
 * Jul 2, 2020
 *
 * Function definitions for `Chunk` `class`
 * as well as operators for the `Pos` `struct`
 *
 */

#include "chunk.h"

Pos operator*(const Pos &p, const double &s) { 
	return Pos {static_cast<long>(p.x * s), static_cast<long>(p.y * s)}; 
}
Pos operator*(const double &s, const Pos &p) { return p * s; }
Pos operator/(const Pos &p, const double &s) { 
	return Pos {static_cast<long>(p.x / s), static_cast<long>(p.y / s)}; 
}
Pos operator/(const double &s, const Pos &p) { return p / s; }
Pos operator+(const Pos &l, const Pos &r) { return Pos {l.x + r.x, l.y + r.y}; }

Chunk::Chunk() {
	reset();
}
void Chunk::reset() {
	n = 0;
	pos.x = 0;
	pos.y = 0;
	mean = 0;
	std = 0;
	var = 0;
}

Chunk Chunk::operator+(const Chunk& r) const {
	Chunk c;
	c.mean = (n * mean + r.n * r.mean) / (n + r.n);
	c.var = ((n - 1) * var + (r.n - 1) * r.var) / (n + r.n - 1) + n * r.n * std::pow(mean - r.mean, 2) / ((n + r.n) * (n + r.n - 1));
	
	c.std = std::sqrt(c.var);
	c.n = n + r.n;
	c.pos = (pos * n + r.pos * r.n) / c.n;

	return c;
}
Chunk& Chunk::operator+=(const Chunk& r) {
	this->mean = (n * mean + r.n * r.mean) / (n + r.n);
	this->var = ((n - 1) * var + (r.n - 1) * r.var) / (n + r.n - 1) + n * r.n * std::pow(mean - r.mean, 2) / ((n + r.n) * (n + r.n - 1));
	
	this->std = std::sqrt(this->var);
	this->n = n + r.n;
	this->pos = (pos * n + r.pos * r.n) / this->n;
	return *this;
}
std::ostream& operator<<(std::ostream &os, const Chunk &c) {
	os.precision(2);
	os << std::fixed << std::right;
	os << "Mean: " << std::setw(5) << c.mean << ",  ";
	os << "var: " << std::setw(7) << c.var << ",  ";
	os << "std: " << std::setw(5) << c.std << ",  ";
	os << "pos: (" << std::setw(5) << c.pos.x << ",  " << std::setw(5) << c.pos.y << "),  ";
	os << "n: " << c.n << " ";
}
