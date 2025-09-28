/**************************************************************************************************
 * Obj reader
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
typedef struct ObjReader_MeshSizes {
    uint32_t nPos;
    uint32_t nNorms;
    uint32_t nTex;
    uint32_t nFaces;
    uint32_t flatFacesSize;
} ObjReader_MeshSizes;

typedef struct ObjReader_MeshData {
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
    uint32_t *facePosIdx;
    uint32_t *faceNormIdx;
    uint32_t *faceTexIdx;

    // Faces offsets in previous 3 datasets
    uint32_t *faceSizes;

} ObjReader_MeshData;

typedef struct ObjReader_Mesh {
    ObjReader_MeshSizes sizes;
    ObjReader_MeshData data;
} ObjReader_Mesh;

typedef struct ObjReader_Return {
    bool successfulRead;
    ObjReader_Mesh mesh;
} ObjReader_Return;

extern ObjReader_Return read_obj(const char *path);

#endif  // OBJ_READER_H
