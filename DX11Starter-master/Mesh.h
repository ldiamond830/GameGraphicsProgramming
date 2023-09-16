#pragma once
#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include "Vertex.h"

class Mesh
{
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> drawCommandPtr;
	UINT numIndices;
	UINT numVertices;

public:
	Mesh(Vertex* vertices, int numVertices, unsigned int* indices, int numIndeces, Microsoft::WRL::ComPtr<ID3D11DeviceContext> drawCommandPtr, Microsoft::WRL::ComPtr <ID3D11Device> bufferCreator);
	~Mesh();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	int GetIndexCount();
	void Draw();


};

