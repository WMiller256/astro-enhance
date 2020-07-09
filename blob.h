/*
 * blob.h
 *
 * William Miller
 * Jul 5, 2020
 *
 * Blob class definition, used to store
 * two vectors of pixel positions which 
 * correspond to the block and nearest
 * neighbor pixels
 *
 */

#pragma once

#include <algorithm>
#include <vector>

#include "chunk.h" // Needed for Pos struct 

class Blob {

public:
    Blob();

    std::vector<Pos> blob;
    std::vector<Pos> perim;

    void to_blob(Pos p);
    void to_perim(Pos p);

    bool blob_contains(Pos p);
    bool perim_contains(Pos p);

};
