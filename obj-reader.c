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
 * Includes
 *************************************************************************************************/

#include "obj-reader.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**************************************************************************************************
 * Macros
 *************************************************************************************************/

#define MAX_LINE_LEN (65535)
#define NUM_LINE_TYPES (12)

/**************************************************************************************************
 * Enums
 *************************************************************************************************/

typedef enum ObjRdr_LineType {
    OBJ_RDR_COMMENT = 0,
    OBJ_RDR_VECPOS = 1,
    OBJ_RDR_VECTEXT = 2,
    OBJ_RDR_VECNORM = 3,
    OBJ_RDR_VECPARAM = 4,
    OBJ_RDR_FACE = 5,
    OBJ_RDR_LINE = 6,
    OBJ_RDR_MTLSPEC = 7,
    OBJ_RDR_MTLUSE = 8,
    OBJ_RDR_OBJECT = 9,
    OBJ_RDR_GROUP = 10,
    OBJ_RDR_SSHADING = 11,
    OBJ_RDR_DEFAULT = 12,
} ObjRdr_LineType;

/**************************************************************************************************
 * Structs
 *************************************************************************************************/

/**************************************************************************************************
 * Constants
 *************************************************************************************************/

static const char *const LINE_SPEC[NUM_LINE_TYPES] = {
    [OBJ_RDR_COMMENT] = "# ",
    [OBJ_RDR_VECPOS] = "v ",
    [OBJ_RDR_VECTEXT] = "vt ",
    [OBJ_RDR_VECNORM] = "vn ",
    [OBJ_RDR_VECPARAM] = "vp ",
    [OBJ_RDR_FACE] = "f ",
    [OBJ_RDR_LINE] = "l ",
    [OBJ_RDR_MTLSPEC] = "mtllib ",
    [OBJ_RDR_MTLUSE] = "usemtl ",
    [OBJ_RDR_OBJECT] = "o ",
    [OBJ_RDR_GROUP] = "g ",
    [OBJ_RDR_SSHADING] = "s ",
};

/**************************************************************************************************
 * Variables
 *************************************************************************************************/

static const char *currentPath;
static char lineBuff[MAX_LINE_LEN];

/**************************************************************************************************
 * Helper methods
 *************************************************************************************************/

static uint32_t strlen_s(const char *s) {
    static const uint32_t MAX_LEN = (uint32_t)0xffffffff;
    for (uint32_t i = 0; i < MAX_LEN; ++i) {
        if (!s) {
            return i;
        }
        ++s;
    }
    return MAX_LEN;
}

static bool str_endswith(const char *s, const char *suff) {
    uint32_t sLen = strlen_s(s);
    uint32_t suffLen = strlen_s(suff);

    return (sLen >= suffLen && strncmp(s + sLen - suffLen, suff, suffLen) == 0);
}

static const bool try_open_obj(FILE **fptr_p) {
    if (!str_endswith(currentPath, ".obj")) {
        fprintf(
            stderr,
            "Error, trying to read file %s:\n\
                not an .obj file",
            currentPath
        );
        return false;
    }

    // Check from path whether the file is indeed Wavefront format

    *fptr_p = fopen(currentPath, "r");

    if (!*fptr_p) {
        fprintf(
            stderr,
            "Error, trying to read file %s:\n\
                Could not open the file.",
            currentPath
        );
        return false;
    }

    return true;
}

static ObjRdr_LineType get_line_type() {
    for (int i = 0; i < NUM_LINE_TYPES; ++i) {
        if (strncmp(lineBuff, LINE_SPEC[i], strlen(LINE_SPEC[i])) == 0) {
            return (ObjRdr_LineType)i;
        }
    }
    return OBJ_RDR_DEFAULT;
}

static uint32_t get_num_verts() {
    // TODO
}

static ObjRdr_MeshSizes get_sizes(FILE *fptr) {
    ObjRdr_MeshSizes sizes = {0, 0, 0, 0, 0};

    while (fgets(lineBuff, MAX_LINE_LEN, fptr)) {
        ObjRdr_LineType lineType = get_line_type();
        switch (lineType) {
            case OBJ_RDR_COMMENT:
                break;
            case OBJ_RDR_VECPOS:
                ++sizes.nPos;
                break;
            case OBJ_RDR_VECTEXT:
                ++sizes.nTex;
                break;
            case OBJ_RDR_VECNORM:
                ++sizes.nNorms;
                break;
            case OBJ_RDR_FACE:
                ++sizes.nFaces;
                sizes.flatFacesSize = get_num_verts();

            // currently unsupported
            // TODO: extend support to objects and groups first, then materials
            case OBJ_RDR_VECPARAM:
            case OBJ_RDR_LINE:
            case OBJ_RDR_MTLSPEC:
            case OBJ_RDR_MTLUSE:
            case OBJ_RDR_OBJECT:
            case OBJ_RDR_GROUP:
            case OBJ_RDR_SSHADING:
            case OBJ_RDR_DEFAULT:
                break;
        }
    }

    return sizes;
}

static bool alloc_mesh_data(ObjRdr_MeshData *data, ObjRdr_MeshSizes sizes) {
    // Vertex position data
    data->posX = malloc(sizes.nPos * sizeof(*data->posX));
    data->posY = malloc(sizes.nPos * sizeof(*data->posY));
    data->posZ = malloc(sizes.nPos * sizeof(*data->posZ));
    data->posW = malloc(sizes.nPos * sizeof(*data->posW));

    if (!data->posX || !data->posY || !data->posZ || !data->posW) {
        fprintf(
            stderr,
            "Error reading the wavefront file %s:\n\
                Failed to allocate memory for the vertex positions",
            currentPath
        );
        return false;
    }

    // Vertex normals data
    data->normX = malloc(sizes.nNorms * sizeof(*data->normX));
    data->normY = malloc(sizes.nNorms * sizeof(*data->normY));
    data->normZ = malloc(sizes.nNorms * sizeof(*data->normZ));

    if (!data->normX || !data->normY || !data->normZ) {
        fprintf(
            stderr,
            "Error reading the wavefront file %s:\n\
                Failed to allocate memory for the vertex normals.",
            currentPath
        );
        return false;
    }

    // Vertex texture coordinates data
    data->texU = malloc(sizes.nTex * sizeof(*data->texU));
    data->texV = malloc(sizes.nTex * sizeof(*data->texV));

    if (!data->texU || !data->texV) {
        fprintf(
            stderr,
            "Error reading the wavefront file %s:\n\
                Failed to allocate memory for the vertex texture coordinates.",
            currentPath
        );
        return false;
    }

    // Polygon vertices
    data->faces = malloc(sizes.flatFacesSize * sizeof(*data->faces));

    // Faces offsets in previous 3 datasets
    data->faceSizes = malloc(sizes.nFaces * sizeof(*data->faceSizes));

    if (!data->faces || !data->faceSizes) {
        fprintf(
            stderr,
            "Error reading the wavefront file %s:\n\
                Failed to allocate memory for the faces data.",
            currentPath
        );
        return false;
    }

    return data;
}

static ObjRdr_MeshData get_data(FILE *fptr, ObjRdr_MeshSizes sizes) {
    ObjRdr_MeshData data;
    alloc_mesh_data(&data, sizes);

    return data;
}

/**************************************************************************************************
 * Public methods
 *************************************************************************************************/

ObjRdr_Return read_obj(const char *path) {
    // TODO: sanitise path (trim if too long and remove %p and other known attacks)
    currentPath = path;

    ObjRdr_Mesh mesh = {};

    FILE *fptr;
    if (!try_open_obj(&fptr)) {
        return (ObjRdr_Return) {false, mesh};
    }

    mesh.sizes = get_sizes(fptr);
    mesh.data = get_data(fptr, mesh.sizes);

    return (ObjRdr_Return) {true, mesh};
}
