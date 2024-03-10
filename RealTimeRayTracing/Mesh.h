#pragma once
#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include "Vertex.h"
#include"DX12Helper.h"

struct MeshRaytracingData
{
	D3D12_GPU_DESCRIPTOR_HANDLE IndexbufferSRV{ };
	D3D12_GPU_DESCRIPTOR_HANDLE VertexBufferSRV{ };
	Microsoft::WRL::ComPtr<ID3D12Resource> BLAS;
	unsigned int HitGroupIndex = 0;
};

class Mesh
{
private:
	UINT numIndices;
	UINT numVertices;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);
	D3D12_VERTEX_BUFFER_VIEW vbView;
	D3D12_INDEX_BUFFER_VIEW ibView;
	MeshRaytracingData raytracingData;

public:
	Mesh(Vertex* vertices, int numVertices, unsigned int* indices, int numIndeces);
	Mesh(const wchar_t* filePath);
	~Mesh();
	Microsoft::WRL::ComPtr<ID3D12Resource> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D12Resource> GetIndexBuffer();
	D3D12_VERTEX_BUFFER_VIEW GetVBView() { return vbView; } // Renamed
	D3D12_INDEX_BUFFER_VIEW GetIBView() { return ibView; } // Renamed
	Microsoft::WRL::ComPtr<ID3D12Resource> GetVBResource() { return vertexBuffer; }
	Microsoft::WRL::ComPtr<ID3D12Resource> GetIBResource() { return indexBuffer; }
	MeshRaytracingData GetRaytracingData() { return raytracingData; }
	int GetIndexCount();
	int GetVertexCount();
	void Draw();
};

