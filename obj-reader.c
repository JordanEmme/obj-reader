#include "obj-reader.h"
#include <stdio.h>
#include <stdlib.h>

static const char *currentPath;

static ObjReader_MeshSizes get_sizes() {
    ObjReader_MeshSizes sizes;
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

    // Vertex texture coordinates data
    data->texU = malloc(sizes.nTex * sizeof(*data->texU));
    data->texV = malloc(sizes.nTex * sizeof(*data->texV));

    // Polygon vertices
    data->facePosIdx = malloc(sizes.flatFacesSize * sizeof(*data->facePosIdx));
    data->faceNormIdx = malloc(sizes.flatFacesSize * sizeof(*data->faceNormIdx));
    data->faceTexIdx = malloc(sizes.flatFacesSize * sizeof(*data->faceTexIdx));

    // Faces offsets in previous 3 datasets
    data->faceSizes = malloc(sizes.nFaces * sizeof(*data->faceSizes));

    return data;
}

static ObjReader_MeshData get_data(ObjReader_MeshSizes sizes) {
    ObjReader_MeshData data;
    alloc_mesh_data(&data, sizes);

    return data;
}

ObjReader_Return read_obj(const char *path) {
    currentPath = path;
    ObjReader_Mesh mesh;
    mesh.sizes = get_sizes(path);

    mesh.data = get_data(mesh.sizes);

    return (ObjReader_Return) {true, mesh};
}
