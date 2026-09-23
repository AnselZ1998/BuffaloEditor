#pragma once
#include <vector>
#include "math/Vector.h"

struct Mesh
{
    // OBJ: v
    std::vector<Vector3> vertices;

    // OBJ: vn
    std::vector<Vector3> normals;

    // OBJ: vt
    std::vector<Vector3> uv;

    std::vector<Vector3i> vertexIndices;
    std::vector<Vector3i> uvIndices;
    std::vector<Vector3i> normalsIndices;

    int numVertices = 0;
    int numFaces = 0;

	bool IsValid() const
	{
		return 
            !vertices.empty() && !vertexIndices.empty() &&
            !normals.empty() && !normalsIndices.empty() &&
            !uv.empty() && !uvIndices.empty();
	}
};