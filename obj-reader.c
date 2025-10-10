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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "obj-reader.h"

/**************************************************************************************************
 * Macros
 *************************************************************************************************/

#define MAX_LINE_LEN (65535)

#define FREE(A) \
    { \
        if (A) { \
            free(A); \
        } \
    }

/**************************************************************************************************
 * Enums
 *************************************************************************************************/

typedef enum Obj_LineType {
    OBJ_COMMENT  = 0,
    OBJ_VECPOS   = 1,
    OBJ_VECTEXT  = 2,
    OBJ_VECNORM  = 3,
    OBJ_VECPARAM = 4,
    OBJ_FACE     = 5,
    OBJ_LINE     = 6,
    OBJ_MTLSPEC  = 7,
    OBJ_MTLUSE   = 8,
    OBJ_OBJECT   = 9,
    OBJ_GROUP    = 10,
    OBJ_SSHADING = 11,

    OBJ_NUM_LINE_TYPES,
    OBJ_INVALID_LINE,
} Obj_LineType;

typedef enum Obj_FaceType {
    OBJ_POS_ONLY         = 0,
    OBJ_POS_AND_TEX      = 1,
    OBJ_POS_AND_NORM     = 2,
    OBJ_POS_TEX_AND_NORM = 3,

    OBJ_NUM_FACE_TYPES,
    OBJ_INVALID_FACE,
} Obj_FaceType;

/**************************************************************************************************
 * Structs
 *************************************************************************************************/

/**************************************************************************************************
 * Constants
 *************************************************************************************************/

static const char *const LINE_SPEC[OBJ_NUM_LINE_TYPES] = {
    [OBJ_COMMENT]  = "# ",
    [OBJ_VECPOS]   = "v ",
    [OBJ_VECTEXT]  = "vt ",
    [OBJ_VECNORM]  = "vn ",
    [OBJ_VECPARAM] = "vp ",
    [OBJ_FACE]     = "f ",
    [OBJ_LINE]     = "l ",
    [OBJ_MTLSPEC]  = "mtllib ",
    [OBJ_MTLUSE]   = "usemtl ",
    [OBJ_OBJECT]   = "o ",
    [OBJ_GROUP]    = "g ",
    [OBJ_SSHADING] = "s ",
};

static const char *const FACE_INDEX_FORMAT[OBJ_NUM_FACE_TYPES] = {
    [OBJ_POS_ONLY]         = "%d",
    [OBJ_POS_AND_TEX]      = "%d/%d",
    [OBJ_POS_AND_NORM]     = "%d//%d",
    [OBJ_POS_TEX_AND_NORM] = "%d/%d/%d",
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
    uint32_t sLen    = strlen_s(s);
    uint32_t suffLen = strlen_s(suff);

    return (sLen >= suffLen && strncmp(s + sLen - suffLen, suff, suffLen) == 0);
}

static const bool try_open_obj(FILE **fptr_p) {
    if (!str_endswith(currentPath, ".obj") && !str_endswith(currentPath, ".OBJ")) {
        fprintf(stderr, "Error, trying to read file %s:\n Not an .obj file\n", currentPath);
        return false;
    }

    // Check from path whether the file is indeed Wavefront format

    *fptr_p = fopen(currentPath, "r");

    if (!*fptr_p) {
        fprintf(stderr, "Error, trying to read file %s:\n Could not open the file.", currentPath);
        return false;
    }

    return true;
}

static Obj_LineType get_line_type() {
    for (int i = 0; i < OBJ_NUM_LINE_TYPES; ++i) {
        if (strncmp(lineBuff, LINE_SPEC[i], strlen(LINE_SPEC[i])) == 0) {
            return (Obj_LineType)i;
        }
    }
    return OBJ_INVALID_LINE;
}

static Obj_MeshSizes get_sizes(FILE *fptr) {
    Obj_MeshSizes sizes = {0u, 0u, 0u, 0u, 0u};

    uint32_t lineNum = 0u;

    while (fgets(lineBuff, MAX_LINE_LEN, fptr)) {
        ++lineNum;
        Obj_LineType lineType = get_line_type();
        switch (lineType) {
            case OBJ_COMMENT:
                break;
            case OBJ_VECPOS:
                ++sizes.nPos;
                break;
            case OBJ_VECTEXT:
                ++sizes.nTex;
                break;
            case OBJ_VECNORM:
                ++sizes.nNorms;
                break;
            case OBJ_FACE:
                ++sizes.nFaces;

                // TODO: Assumes there is no double space or trailing spaces before the newline, should make it
                // more robust
                for (int i = 0; i < strlen(lineBuff); ++i) {
                    sizes.flatFacesSize += lineBuff[i] == ' ';
                }
                break;
            case OBJ_INVALID_LINE:
                fprintf(stderr, "Error: line %d is not recognized:\n > %s", lineNum, lineBuff);
            // currently unsupported
            // TODO: extend support to objects and groups first, then materials
            case OBJ_VECPARAM:
            case OBJ_LINE:
            case OBJ_MTLSPEC:
            case OBJ_MTLUSE:
            case OBJ_OBJECT:
            case OBJ_GROUP:
            case OBJ_SSHADING:
            default:
                break;
        }
    }

    return sizes;
}

static bool alloc_mesh_data(Obj_MeshData *data, Obj_MeshSizes sizes) {
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

static Obj_FaceType determine_face_type() {
    //TODO: make this robust to contiguous whitespaces, and make it detect invalid formats
    int  i = 2;  // ignore 'f' and first space start from first digit
    char c;
    bool justParsedSlash = false;
    bool parsedSlash     = false;

    while (lineBuff[i] != ' ') {
        c = lineBuff[i];
        if (c - '0' < 10) {
            justParsedSlash = false;
            continue;
        } else if (c == '/') {
            if (justParsedSlash) {
                return OBJ_POS_AND_NORM;
            } else if (parsedSlash) {
                return OBJ_POS_TEX_AND_NORM;
            }
            parsedSlash     = true;
            justParsedSlash = true;
        }
        ++i;
    }
    return parsedSlash ? OBJ_POS_AND_TEX : OBJ_POS_ONLY;
}

static uint32_t parse_face(Obj_MeshData *data, uint32_t faceVtxIdx) {
    // Determine if only pos, pos and norm, pos and text, or all three
    Obj_FaceType faceType   = determine_face_type();
    const char  *faceFormat = FACE_INDEX_FORMAT[faceType];

    uint32_t numVertices = 0u;
    int32_t  posIdx      = -1;
    int32_t  normIdx     = -1;
    int32_t  texIdx      = -1;

    char *intBuff = strtok(lineBuff + 2, " ");
    while (intBuff) {
        switch (faceType) {
            case OBJ_POS_ONLY:
                sscanf(intBuff, faceFormat, &posIdx);
                break;
            case OBJ_POS_AND_TEX:
                sscanf(intBuff, faceFormat, &posIdx, &texIdx);
                break;
            case OBJ_POS_AND_NORM:
                sscanf(intBuff, faceFormat, &posIdx, &normIdx);
                break;
            case OBJ_POS_TEX_AND_NORM:
                sscanf(intBuff, faceFormat, &posIdx, &texIdx, &normIdx);
                break;
            case OBJ_INVALID_FACE:
            default:
                fprintf(stderr, "Error parsing face n%d", faceVtxIdx);
                return 0;
        }
        data->faces[faceVtxIdx + numVertices].posIdx  = posIdx;
        data->faces[faceVtxIdx + numVertices].texIdx  = texIdx;
        data->faces[faceVtxIdx + numVertices].normIdx = normIdx;
        ++numVertices;
    }
    return numVertices;
}

static Obj_MeshData try_get_data(FILE *fptr, Obj_MeshSizes sizes, bool *successfulRead) {
    Obj_MeshData data;
    alloc_mesh_data(&data, sizes);

    uint32_t posIdx  = 0u;
    uint32_t normIdx = 0u;
    uint32_t textIdx = 0u;

    uint32_t faceVtxIdx  = 0u;
    uint32_t faceIdx     = 0u;
    uint32_t numVertices = 0u;

    uint32_t lineNum = 0u;

    while (fgets(lineBuff, MAX_LINE_LEN, fptr)) {
        ++lineNum;
        switch (get_line_type()) {
            case OBJ_VECPOS:
                if (sscanf(
                        lineBuff,
                        "v %f %f %f",
                        data.posX + posIdx,
                        data.posY + posIdx,
                        data.posZ + posIdx
                    )) {
                    data.posW[posIdx] = 1.0f;
                    ++posIdx;
                } else if (sscanf(
                               lineBuff,
                               "v %f %f %f %f",
                               data.posX + posIdx,
                               data.posY + posIdx,
                               data.posZ + posIdx,
                               data.posW + posIdx
                           )) {
                    ++posIdx;
                } else {
                    fprintf(
                        stderr,
                        "Error, line %d, vector position line in invalid format:\n > %s",
                        lineNum,
                        lineBuff
                    );
                    return data;
                }
                break;
            case OBJ_VECNORM:
                if (sscanf(
                        lineBuff,
                        "vn %f %f %f",
                        data.normX + normIdx,
                        data.normY + normIdx,
                        data.normZ + normIdx
                    )) {
                    ++normIdx;
                } else {
                    fprintf(
                        stderr,
                        "Error, line %d, vector normal line in invalid format:\n > %s",
                        lineNum,
                        lineBuff
                    );
                    return data;
                }
                break;
            case OBJ_VECTEXT:
                if (sscanf(lineBuff, "vt %f %f", data.texU + textIdx, data.texV + textIdx)) {
                    ++textIdx;
                } else {
                    fprintf(
                        stderr,
                        "Error, line %d, vector texture coordinates line in invalid format:\n > %s",
                        lineNum,
                        lineBuff
                    );
                    return data;
                }
                break;
            case OBJ_FACE:
                numVertices = parse_face(&data, faceVtxIdx);

                if (numVertices < 3) {
                    fprintf(
                        stderr,
                        "Error, obj file %s contains a face with fewer than 3 vertices",
                        currentPath
                    );
                }
                data.faceSizes[faceIdx++] = numVertices;
                faceVtxIdx += numVertices;
                break;

            case OBJ_COMMENT:
            case OBJ_VECPARAM:
            case OBJ_LINE:
            case OBJ_MTLSPEC:
            case OBJ_MTLUSE:
            case OBJ_OBJECT:
            case OBJ_GROUP:
            case OBJ_SSHADING:
            case OBJ_INVALID_LINE:
            default:
                break;
        }
    }
    *successfulRead = true;
    return data;
}

/**************************************************************************************************
 * Public methods
 *************************************************************************************************/

void obj_free(Obj_Mesh *mesh) {
    FREE(mesh->data.posX);
    FREE(mesh->data.posY);
    FREE(mesh->data.posZ);
    FREE(mesh->data.posW);
    FREE(mesh->data.normX);
    FREE(mesh->data.normY);
    FREE(mesh->data.normZ);
    FREE(mesh->data.texU);
    FREE(mesh->data.texV);
    FREE(mesh->data.faces);
    FREE(mesh->data.faceSizes);
}

Obj_Return obj_read(const char *path) {
    // TODO: sanitise path (trim if too long and remove %p and other known attacks)
    currentPath = path;

    Obj_Mesh mesh = {};

    FILE *fptr;
    if (!try_open_obj(&fptr)) {
        return (Obj_Return) {false, mesh};
    }

    fprintf(stdout, "Opened obj file %s for reading\n", currentPath);

    mesh.sizes = get_sizes(fptr);
    rewind(fptr);

    bool successfulRead = false;

    mesh.data = try_get_data(fptr, mesh.sizes, &successfulRead);
    fclose(fptr);
    if (!successfulRead) {
        fprintf(stderr, "Error while reading the obj file, aborting the operation.");
    }

    return (Obj_Return) {successfulRead, mesh};
}
