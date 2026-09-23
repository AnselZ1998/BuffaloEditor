#include "math/BoundingVolume.h"

void BoundingVolume::GetBoundingVolume(const Vector4& inVectorPa, const Vector4& inVectorPb, const Vector4& inVectorPc)
{
	Vector3 min = Vector4::GetMin(inVectorPa, inVectorPb, inVectorPc);
	Vector3 max = Vector4::GetMax(inVectorPa, inVectorPb, inVectorPc);
	minX = min.x;
	minY = min.y;
	maxX = max.x;
	maxY = max.y;
}