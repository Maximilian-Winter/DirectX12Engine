#include "StaticMesh.h"

#include "Src/Direct3D.h"
#include "Src/MaterialManager.h"

StaticMesh::StaticMesh()
{
}

StaticMesh::StaticMesh(Direct3D* d3d12Device, std::vector<StaticVertex> vertices, std::vector<int> indicies, std::vector<Subset> subsets)
{
	CreateBuffers(d3d12Device, vertices, indicies, subsets);
}

StaticMesh::StaticMesh(const StaticMesh&)
{

}

StaticMesh::~StaticMesh()
{

}

void StaticMesh::CreateBuffers(Direct3D* d3d12Device, std::vector<StaticVertex> vertices, std::vector<int> indicies, std::vector<Subset> subsets)
{
	
	//MeshUtilities::Vertex* Vertices = new MeshUtilities::Vertex[vertices.size()];
	//std::uint32_t* Indicies = new std::uint32_t[indicies.size()];

	vbByteSize = (UINT)vertices.size() * sizeof(StaticVertex);
	ibByteSize = (UINT)indicies.size() * sizeof(std::uint32_t);

	m_MeshGeometry = std::make_unique<MeshUtilities::MeshGeometry>();

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &m_MeshGeometry->VertexBufferCPU));
	CopyMemory(m_MeshGeometry->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &m_MeshGeometry->IndexBufferCPU));
	CopyMemory(m_MeshGeometry->IndexBufferCPU->GetBufferPointer(), indicies.data(), ibByteSize);

	d3d12Device->BeginCommandListRecording();
	m_MeshGeometry->VertexBufferGPU = DefaultBufferCreator::CreateDefaultBuffer(d3d12Device,
		vertices.data(), vbByteSize, m_MeshGeometry->VertexBufferUploader);

	m_MeshGeometry->IndexBufferGPU = DefaultBufferCreator::CreateDefaultBuffer(d3d12Device
		, indicies.data(), ibByteSize, m_MeshGeometry->IndexBufferUploader);

	d3d12Device->StopCommandListRecordingAndExecute();
	d3d12Device->WaitForGpuToFinishWork();

	m_MeshGeometry->VertexByteStride = sizeof(StaticVertex);
	m_MeshGeometry->VertexBufferByteSize = vbByteSize;
	m_MeshGeometry->IndexFormat = DXGI_FORMAT_R32_UINT;
	m_MeshGeometry->IndexBufferByteSize = ibByteSize;

	int subsetIndex = 0;
	for (const auto element : subsets)
	{
		MeshUtilities::SubmeshGeometry submesh;
		submesh.IndexCount = (UINT)element.drawAmount;
		submesh.StartIndexLocation = element.start;
		submesh.BaseVertexLocation = 0;
		submesh.MaterialName = element.materialName;
		m_MeshGeometry->DrawArgs[subsetIndex] = submesh;
		subsetIndex++;
	}
}

void StaticMesh::Render(Direct3D* direct3d)
{
	m_IndexBufferView = m_MeshGeometry->IndexBufferView();
	m_VertexBufferView = m_MeshGeometry->VertexBufferView();
	direct3d->GetCommandList()->IASetIndexBuffer(&m_IndexBufferView);
	direct3d->GetCommandList()->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	direct3d->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (auto element : m_MeshGeometry->DrawArgs)
	{
		int albedoIndex = MaterialManager::Get()->GetMaterial(element.second.MaterialName).AlbedoMapIndex;
		if(albedoIndex >= 0)
		{
			direct3d->SetSRV(MaterialManager::Get()->GetMaterial(element.second.MaterialName).Textures[albedoIndex].GetSRV());
		}
		direct3d->SetMeshDataHeaps();
		direct3d->GetCommandList()->DrawIndexedInstanced(element.second.IndexCount, 1, element.second.StartIndexLocation, element.second.BaseVertexLocation,0);
	}

}
