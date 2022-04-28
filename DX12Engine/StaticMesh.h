#ifndef _STATICMESH_H_
#define _STATICMESH_H_
#include <d3d12.h>
#include <directxmath.h>
#include <string>
#include <vector>
#include <wrl/client.h>

#include "Src/MaterialManager.h"
#include "Src/TransformNode.h"
#include "Src/MeshUtilities.h"

struct Subset
{
	Subset() :
		materialName(""),
		start(0),
		drawAmount(0)
	{
	}

	Subset(std::string materialName, int start, int drawAmount)
		:materialName(materialName), start(start), drawAmount(drawAmount)
	{
	}
	std::string materialName;
	int start;
	int drawAmount;
};

struct StaticVertex
{
	StaticVertex() : x(0), y(0), z(0), tu(0), tv(0), nx(0), ny(0), nz(0)
	{
	}

	StaticVertex(float x, float y, float z, float tu, float tv, float nx, float ny, float nz)
		:x(x), y(y), z(z), tu(tu), tv(tv), nx(nx), ny(ny), nz(nz)
	{}

	float x, y, z;
	float tu, tv;
	float nx, ny, nz;
};

class StaticMesh
{
public:
	StaticMesh();
	StaticMesh(Direct3D* d3d12Device, std::vector<StaticVertex> vertices, std::vector<int> indicies, std::vector<Subset> subsets);
	StaticMesh(const StaticMesh&);
	~StaticMesh();

	void CreateBuffers(Direct3D* d3d12Device, std::vector<StaticVertex> vertices, std::vector<int> indicies, std::vector<Subset> subsets);
	void Render(Direct3D* direct3d);
private:

	std::unique_ptr<MeshUtilities::MeshGeometry> m_MeshGeometry;
	DirectX::BoundingOrientedBox MeshBoundingBox;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

	UINT vbByteSize;
	UINT ibByteSize;
};
#endif
