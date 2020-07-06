/*
 * blob.c++
 *
 * William Miller
 * Jul 5, 2020
 *
 * Implementation for Blob class
 *
 */

#include "blob.h"

Blob::Blob() {}

void Blob::to_blob(Pos p) { if (!blob_contains(p)) blob.push_back(p); }
void Blob::to_perim(Pos p) { if (!perim_contains(p)) perim.push_back(p); }

bool Blob::blob_contains(Pos p) { return (std::find(blob.begin(), blob.end(), p) != v.end()); }
bool Blob::perim_contains(Pos p) { return (std::find(perim.begin(), perim.end(), p) != v.end()); }
