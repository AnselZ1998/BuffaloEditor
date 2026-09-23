#include "assets/model/Model.h"
#include "assets/import/objParser.h"


Model::Model(
    const std::string& meshPath,
    const Matrix4& meshMatrix
)
{
    mModelMatrix = meshMatrix;

    IsValid = OBJ::buildMeshFromFile(
        mMesh,
        meshPath
    );
}


const Mesh* Model::GetMesh() const
{
    return &mMesh;
}


const Matrix4& Model::GetModelMatrix() const
{
    return mModelMatrix;
}


void Model::SetModelMatrix(const Matrix4& matrix)
{
    mModelMatrix = matrix;
}


Material* Model::GetMaterial()
{
    return &mMaterial;
}


const bool Model::MeshValid()
{
    return IsValid;
}