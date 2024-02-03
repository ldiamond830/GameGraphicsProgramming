#pragma once
#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include "Vertex.h"
#include"DX12Helper.h"
class Mesh
{
private:
	UINT numIndices;
	UINT numVertices;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);

public:
	Mesh(Vertex* vertices, int numVertices, unsigned int* indices, int numIndeces);
	Mesh(const wchar_t* filePath);
	~Mesh();
	Microsoft::WRL::ComPtr<ID3D12Resource> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D12Resource> GetIndexBuffer();
	int GetIndexCount();
	int GetVertexCount();
	void Draw();
};

