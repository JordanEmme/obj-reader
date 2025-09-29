/**************************************************************************************************
 * Obj Reader
 *
 * File: obj_reader.h
 *
 * Author: Jordan Emme
 *
 * Description: A minimal library that enables reading wavefront (.obj) files.
 *
 *    This is a tiny helper which only enables parsing through the file to hold what is essentially
 * a binary representation of it, and which should be easier to work with/get started. Any
 * re-ordering of the data, such as storing vertices in their own structs/classes using things like
 * float3s or vec3s is left for the user to do. 
 *    Currently, only vertex positions, normals, texture coordinates and faces are supported, and
 * only if the .obj doesn't use relative indices. The author is planning to support groups, objects
 * material declaration and usage, and lines.
 *
 * Copyright (c) 2025 Jordan Emme
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions: The above copyright notice and this
 * permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
*************************************************************************************************/

/**************************************************************************************************
* Header Guard
*************************************************************************************************/

#ifndef OBJ_READER_H
#define OBJ_READER_H

/**************************************************************************************************
* Includes
*************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/**************************************************************************************************
* Definitions
*************************************************************************************************/

/*
 * ObjReader_MeshSizes:
 *
 * Stores the sizes of the various objects that make up the mesh
 * @nPos: number of vertex positions
 * @nNorms: number of normal vectors 
 * @nTex: number of texture coordinates
 * @nFaces: number of faces in the mesh
 * @flatFacesSize: size of the flattened faces array 
 */
typedef struct ObjRdr_MeshSizes {
    uint32_t nPos;
    uint32_t nNorms;
    uint32_t nTex;
    uint32_t nFaces;
    uint32_t flatFacesSize;
} ObjRdr_MeshSizes;

typedef struct ObjRdr_VertIdx {
    int32_t posIdx;
    int32_t normIdx;
    int32_t texIdx;
} ObjRdr_VertIdx;

typedef struct ObjRdr_MeshData {
    // Vertex position data
    float *posX;
    float *posY;
    float *posZ;
    float *posW;

    // Vertex normals data
    float *normX;
    float *normY;
    float *normZ;

    // Vertex texture coordinates data
    float *texU;
    float *texV;

    // Polygon vertices
    ObjRdr_VertIdx *faces;

    // Faces offsets in previous 3 datasets
    uint32_t *faceSizes;

} ObjRdr_MeshData;

typedef struct ObjReader_Mesh {
    ObjRdr_MeshSizes sizes;
    ObjRdr_MeshData data;
} ObjRdr_Mesh;

typedef struct ObjRdr_Return {
    bool successfulRead;
    ObjRdr_Mesh mesh;
} ObjRdr_Return;

extern ObjRdr_Return read_obj(const char *path);

#endif  // OBJ_READER_H
