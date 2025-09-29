# Obj Reader

(This is a work in progress)

This is a tiny helper which only enables parsing through the file to hold what is essentially a
binary representation of it, and which should be easier to work with/get started. Any re-ordering of
the data, such as storing vertices in their own structs/classes using things like float3s or vec3s
is left for the user to do.

Currently, only vertex positions, normals, texture coordinates and faces are supported, and only if
the .obj doesn't use relative indices. The author is planning to support groups, objects material
declaration and usage, and lines.
