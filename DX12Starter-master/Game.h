#pragma once

#include "DXCore.h"
#include "Camera.h"
#include "Entity.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory>
#include <vector>
#include "Light.h"
class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void CreateRootSigAndPipelineState();
	void CreateGeometry();
	void CreateMaterials();
	void CreateLights();
	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;

	D3D12_VERTEX_BUFFER_VIEW vbView;
	D3D12_INDEX_BUFFER_VIEW ibView;

	std::shared_ptr<Camera> mainCamera;
	std::vector<std::shared_ptr<Entity>> entityList;
	std::vector<std::shared_ptr<Material>> materialList;
	std::vector<Light> lights;
	int lightCount = 5;

	Light directionalLight1 = {};
	Light directionalLight2 = {};
	Light directionalLight3 = {};
	Light pointLight1 = {};
	Light pointLight2 = {};
};

