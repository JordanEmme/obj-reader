#include "obj-reader.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN (65535)

static const char *currentPath;
static char lineBuff[MAX_LINE_LEN];

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

static ObjReader_MeshSizes get_sizes(FILE *fptr) {
    ObjReader_MeshSizes sizes = {0, 0, 0, 0, 0};

    while (fgets(lineBuff, MAX_LINE_LEN, fptr)) {
        if (strncmp(lineBuff, "v ", 2)) {}
    }

    return sizes;
}

static bool alloc_mesh_data(ObjReader_MeshData *data, ObjReader_MeshSizes sizes) {
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
    data->facePosIdx = malloc(sizes.flatFacesSize * sizeof(*data->facePosIdx));
    data->faceNormIdx = malloc(sizes.flatFacesSize * sizeof(*data->faceNormIdx));
    data->faceTexIdx = malloc(sizes.flatFacesSize * sizeof(*data->faceTexIdx));

    // Faces offsets in previous 3 datasets
    data->faceSizes = malloc(sizes.nFaces * sizeof(*data->faceSizes));

    if (!data->facePosIdx || !data->faceNormIdx || !data->faceTexIdx || !data->faceSizes) {
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

static ObjReader_MeshData get_data(FILE *fptr, ObjReader_MeshSizes sizes) {
    ObjReader_MeshData data;
    alloc_mesh_data(&data, sizes);

    return data;
}

ObjReader_Return read_obj(const char *path) {
    // TODO: sanitise path (trim if too long and remove %p and other known attacks)
    currentPath = path;

    ObjReader_Mesh mesh = {};

    FILE *fptr;
    if (!try_open_obj(&fptr)) {
        return (ObjReader_Return) {false, mesh};
    }

    mesh.sizes = get_sizes(fptr);
    mesh.data = get_data(fptr, mesh.sizes);

    return (ObjReader_Return) {true, mesh};
}
