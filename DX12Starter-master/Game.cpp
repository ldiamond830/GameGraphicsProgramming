#include "Game.h"
#include "Vertex.h"
#include "DX12Helper.h"
#include "Input.h"
#include "PathHelpers.h"
#include "BufferStructs.h"
// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// Direct3D itself, and our window, are not ready at this point!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,			// The application's handle
		L"DirectX Game",	// Text for the window's title bar (as a wide-character string)
		1280,				// Width of the window's client area
		720,				// Height of the window's client area
		false,				// Sync the framerate to the monitor refresh? (lock framerate)
		true)				// Show extra stats (fps) in title bar?
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif

	ibView = {};
	vbView = {};
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Delete all objects manually created within this class
//  - Release() all Direct3D objects created within this class
// --------------------------------------------------------
Game::~Game()
{
	// We need to wait here until the GPU
	// is actually done with its work
	DX12Helper::GetInstance().WaitForGPU();
}

// --------------------------------------------------------
// Called once per program, after Direct3D and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	mainCamera = std::make_shared<Camera>(Camera(((float)this->windowWidth / this->windowHeight), XMFLOAT3(0.0f, 0.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 45, 0.1f, 500.0f, 0.01f, 5.0f));

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	CreateRootSigAndPipelineState();
	CreateMaterials();
	CreateGeometry();
	CreateLights();
}

// --------------------------------------------------------
// Loads the two basic shaders, then creates the root signature 
// and pipeline state object for our very basic demo.
// --------------------------------------------------------
void Game::CreateRootSigAndPipelineState()
{
	// Blobs to hold raw shader byte code used in several steps below
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderByteCode;

	// Load shaders
	{
		// Read our compiled vertex shader code into a blob
		// - Essentially just "open the file and plop its contents here"
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), vertexShaderByteCode.GetAddressOf());
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), pixelShaderByteCode.GetAddressOf());
	}

	// Input layout
	const unsigned int inputElementCount = 4;
	D3D12_INPUT_ELEMENT_DESC inputElements[inputElementCount] = {};
	{
		// Create an input layout that describes the vertex format
		// used by the vertex shader we're using
		//  - This is used by the pipeline to know how to interpret the raw data
		//     sitting inside a vertex buffer

		// Set up the first element - a position, which is 3 float values
		inputElements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT; // How far into the vertex is this?  Assume it's after the previous element
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;		// Most formats are described as color channels, really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";					// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].SemanticIndex = 0;							// This is the 0th position (there could be more)
		
		// Set up the third element - a normal, which is 3 more float values
		inputElements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;	// After the previous element
		inputElements[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;		// 3x 32-bit floats
		inputElements[1].SemanticName = "NORMAL";					// Match our vertex shader input!
		inputElements[1].SemanticIndex = 0;							// This is the 0th normal (there could be more)

		// Set up the second element - a UV, which is 2 more float values
		inputElements[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;	// After the previous element
		inputElements[2].Format = DXGI_FORMAT_R32G32_FLOAT;			// 2x 32-bit floats
		inputElements[2].SemanticName = "TEXCOORD";					// Match our vertex shader input!
		inputElements[2].SemanticIndex = 0;							// This is the 0th uv (there could be more)


		// Set up the fourth element - a tangent, which is 2 more float values
		inputElements[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;	// After the previous element
		inputElements[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;		// 3x 32-bit floats
		inputElements[3].SemanticName = "TANGENT";					// Match our vertex shader input!
		inputElements[3].SemanticIndex = 0;							// This is the 0th tangent (there could be more)
	}

	// Root Signature
	{
		// Describe the range of CBVs needed for the vertex shader
		D3D12_DESCRIPTOR_RANGE cbvRangeVS = {};
		cbvRangeVS.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvRangeVS.NumDescriptors = 1;
		cbvRangeVS.BaseShaderRegister = 0;
		cbvRangeVS.RegisterSpace = 0;
		cbvRangeVS.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Describe the range of CBVs needed for the pixel shader
		D3D12_DESCRIPTOR_RANGE cbvRangePS = {};
		cbvRangePS.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvRangePS.NumDescriptors = 1;
		cbvRangePS.BaseShaderRegister = 0;
		cbvRangePS.RegisterSpace = 0;
		cbvRangePS.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Create a range of SRV's for textures
		D3D12_DESCRIPTOR_RANGE srvRange = {};
		srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srvRange.NumDescriptors = 4;		// Set to max number of textures at once (match pixel shader!)
		srvRange.BaseShaderRegister = 0;	// Starts at s0 (match pixel shader!)
		srvRange.RegisterSpace = 0;
		srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Create the root parameters
		D3D12_ROOT_PARAMETER rootParams[3] = {};

		// CBV table param for vertex shader
		rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[0].DescriptorTable.pDescriptorRanges = &cbvRangeVS;

		// CBV table param for pixel shader
		rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[1].DescriptorTable.pDescriptorRanges = &cbvRangePS;

		// SRV table param
		rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParams[2].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[2].DescriptorTable.pDescriptorRanges = &srvRange;

		// Create a single static sampler (available to all pixel shaders at the same slot)
		// Note: This is in lieu of having materials have their own samplers for this demo
		D3D12_STATIC_SAMPLER_DESC anisoWrap = {};
		anisoWrap.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.Filter = D3D12_FILTER_ANISOTROPIC;
		anisoWrap.MaxAnisotropy = 16;
		anisoWrap.MaxLOD = D3D12_FLOAT32_MAX;
		anisoWrap.ShaderRegister = 0;  // register(s0)
		anisoWrap.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		D3D12_STATIC_SAMPLER_DESC samplers[] = { anisoWrap };

		// Describe and serialize the root signature
		D3D12_ROOT_SIGNATURE_DESC rootSig = {};
		rootSig.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		rootSig.NumParameters = ARRAYSIZE(rootParams);
		rootSig.pParameters = rootParams;
		rootSig.NumStaticSamplers = ARRAYSIZE(samplers);
		rootSig.pStaticSamplers = samplers;

		ID3DBlob* serializedRootSig = 0;
		ID3DBlob* errors = 0;

		D3D12SerializeRootSignature(
			&rootSig,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&serializedRootSig,
			&errors);

		// Check for errors during serialization
		if (errors != 0)
		{
			OutputDebugString((wchar_t*)errors->GetBufferPointer());
		}

		// Actually create the root sig
		device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(rootSignature.GetAddressOf()));
	}

	// Pipeline state
	{
		// Describe the pipeline state
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

		// -- Input assembler related ---
		psoDesc.InputLayout.NumElements = inputElementCount;
		psoDesc.InputLayout.pInputElementDescs = inputElements;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		// Overall primitive topology type (triangle, line, etc.) is set here 
		// IASetPrimTop() is still used to set list/strip/adj options
		// See: https://docs.microsoft.com/en-us/windows/desktop/direct3d12/managing-graphics-pipeline-state-in-direct3d-12

		// Root sig
		psoDesc.pRootSignature = rootSignature.Get();

		// -- Shaders (VS/PS) --- 
		psoDesc.VS.pShaderBytecode = vertexShaderByteCode->GetBufferPointer();
		psoDesc.VS.BytecodeLength = vertexShaderByteCode->GetBufferSize();
		psoDesc.PS.pShaderBytecode = pixelShaderByteCode->GetBufferPointer();
		psoDesc.PS.BytecodeLength = pixelShaderByteCode->GetBufferSize();

		// -- Render targets ---
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;

		// -- States ---
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		psoDesc.RasterizerState.DepthClipEnable = true;

		psoDesc.DepthStencilState.DepthEnable = true;
		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

		psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		// -- Misc ---
		psoDesc.SampleMask = 0xffffffff;

		// Create the pipe state object
		device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pipelineState.GetAddressOf()));
	}
}

// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red	= XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green	= XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue	= XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);


	std::shared_ptr<Mesh> sphere = std::make_shared<Mesh>(Mesh(FixPath(L"../../Assets/Models/sphere.obj").c_str()));
	std::shared_ptr<Mesh> helix = std::make_shared<Mesh>(Mesh(FixPath(L"../../Assets/Models/helix.obj").c_str()));
	std::shared_ptr<Mesh> cube = std::make_shared<Mesh>(Mesh(FixPath(L"../../Assets/Models/cube.obj").c_str()));

	entityList.push_back(std::make_shared<Entity>(Entity(sphere)));
	entityList[0]->SetMaterial(materialList[0]);
	entityList[0]->GetTransform()->SetPosition(2.5f, 0.0f, 0.0f);

	entityList.push_back(std::make_shared<Entity>(Entity(helix)));
	entityList[1]->SetMaterial(materialList[1]);
	entityList[1]->GetTransform()->SetPosition(-2.5f, 0.0f, 0.0f);

	entityList.push_back(std::make_shared<Entity>(Entity(cube)));
	entityList[2]->SetMaterial(materialList[2]);
}

void Game::CreateMaterials()
{

	D3D12_CPU_DESCRIPTOR_HANDLE bronzeAlbedo = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/PBR/bronze_albedo.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE bronzeNormal = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/PBR/bronze_normals.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE bronzeMetal = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/PBR/bronze_metal.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE bronzeRough = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/PBR/bronze_roughness.png").c_str());
	materialList.push_back(std::make_shared<Material>(Material(pipelineState, XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f))));
	materialList[0]->AddTexture(bronzeAlbedo, 0);
	materialList[0]->AddTexture(bronzeNormal, 1);
	materialList[0]->AddTexture(bronzeRough, 2);
	materialList[0]->AddTexture(bronzeMetal, 3);
	materialList[0]->FinalizeMaterial();


	D3D12_CPU_DESCRIPTOR_HANDLE cobblestoneAlbedo = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/PBR/cobblestone_albedo.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE cobblestoneNormal = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/PBR/cobblestone_normals.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE cobblestoneMetal = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/PBR/cobblestone_metal.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE cobblestoneRough = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/PBR/cobblestone_roughness.png").c_str());
	materialList.push_back(std::make_shared<Material>(Material(pipelineState, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2(5.0f, 5.0f), XMFLOAT2(1.0f, 1.0f))));
	materialList[1]->AddTexture(cobblestoneAlbedo, 0);
	materialList[1]->AddTexture(cobblestoneNormal, 1);
	materialList[1]->AddTexture(cobblestoneRough, 2);
	materialList[1]->AddTexture(cobblestoneMetal, 3);
	materialList[1]->FinalizeMaterial();

	D3D12_CPU_DESCRIPTOR_HANDLE floorAlbedo = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/PBR/floor_albedo.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE floorNormal = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/PBR/floor_normals.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE floorMetal = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/PBR/floor_metal.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE floorRough = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/PBR/floor_roughness.png").c_str());
	materialList.push_back(std::make_shared<Material>(Material(pipelineState, XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f))));
	materialList[2]->AddTexture(floorAlbedo, 0);
	materialList[2]->AddTexture(floorNormal, 1);
	materialList[2]->AddTexture(floorRough, 2);
	materialList[2]->AddTexture(floorMetal, 3);
	materialList[2]->FinalizeMaterial();
}

void Game::CreateLights()
{
	
	directionalLight1.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight1.direction = XMFLOAT3(1.0f, -0.25f, 0.0f);
	directionalLight1.color = XMFLOAT3(1.0f, 0.0f, 0.0f);
	directionalLight1.intensity = 1.0f;
	lights.push_back(directionalLight1);

	
	directionalLight2.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight2.direction = XMFLOAT3(-1.0f, 0.0f, 0.0f);
	directionalLight2.color = XMFLOAT3(1.0f, 0.0f, 0.0f);
	directionalLight2.intensity = 1.0f;
	lights.push_back(directionalLight2);

	
	directionalLight3.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight3.direction = XMFLOAT3(0.0f, 1.0f, 0.0f);
	directionalLight3.color = XMFLOAT3(0.0f, 1.0f, 0.0f);
	directionalLight3.intensity = 1.0f;
	lights.push_back(directionalLight3);

	
	pointLight1.type = LIGHT_TYPE_POINT;
	pointLight1.position = XMFLOAT3(-6.0f, 0.0f, 0.0f);
	pointLight1.color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	pointLight1.intensity = 1.0f;
	pointLight1.range = 5.0f;
	lights.push_back(pointLight1);

	
	pointLight2.type = LIGHT_TYPE_POINT;
	pointLight2.position = XMFLOAT3(0.0f, 0.0f, 1.0f);
	pointLight2.color = XMFLOAT3(1.0f, 1.0f, 0.0f);
	pointLight2.intensity = 1.0f;
	pointLight2.range = 10.0f;
	lights.push_back(pointLight2);
}


// --------------------------------------------------------
// Handle resizing to match the new window size.
//  - DXCore needs to resize the back buffer
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();
	mainCamera->UpdateProjectionMatrix((float)this->windowWidth / this->windowHeight);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();

	mainCamera->Update(deltaTime);
	
	for (std::shared_ptr<Entity> e : entityList) {
		//e->GetTransform()->Rotate(0, 5 * deltaTime, 0);
	}
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap =
		DX12Helper::GetInstance().GetCBVSRVDescriptorHeap();
	commandList->SetDescriptorHeaps(1, descriptorHeap.GetAddressOf());

	// Grab the current back buffer for this frame
	Microsoft::WRL::ComPtr<ID3D12Resource> currentBackBuffer = backBuffers[currentSwapBuffer];

	// Clearing the render target
	{
		// Transition the back buffer from present to render target
		D3D12_RESOURCE_BARRIER rb = {};
		rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		rb.Transition.pResource = currentBackBuffer.Get();
		rb.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		rb.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		commandList->ResourceBarrier(1, &rb);

		// Background color (Cornflower Blue in this case) for clearing
		float color[] = { 0.4f, 0.6f, 0.75f, 1.0f };

		// Clear the RTV
		commandList->ClearRenderTargetView(
			rtvHandles[currentSwapBuffer],
			color,
			0, 0); // No scissor rectangles

		// Clear the depth buffer, too
		commandList->ClearDepthStencilView(
			dsvHandle,
			D3D12_CLEAR_FLAG_DEPTH,
			1.0f,	// Max depth = 1.0f
			0,		// Not clearing stencil, but need a value
			0, 0);	// No scissor rects
	}

	// Rendering here!
	{
		// Set overall pipeline state
		commandList->SetPipelineState(pipelineState.Get());

		// Root sig (must happen before root descriptor table)
		commandList->SetGraphicsRootSignature(rootSignature.Get());

		// Set up other commands for rendering
		commandList->OMSetRenderTargets(1, &rtvHandles[currentSwapBuffer], true, &dsvHandle);
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);
		
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Draw
		for (std::shared_ptr<Entity> e : entityList) {
			VertexShaderExternalData vsData = {};
			vsData.projection = mainCamera->GetProjection();
			vsData.view = mainCamera->GetView();
			vsData.world = e->GetTransform()->GetWorldMatrix();
			vsData.worldInverseTranspose = e->GetTransform()->GetWorldInverseTransposeMatrix();

			auto handle = DX12Helper::GetInstance().FillNextConstantBufferAndGetGPUDescriptorHandle(&vsData, sizeof(vsData));
			commandList->SetGraphicsRootDescriptorTable(0, handle);

			PixelShaderExternalData psData = {};
			psData.uvScale = e->GetMaterial()->GetUVScale();
			psData.uvOffset = e->GetMaterial()->GetUVOffset();
			psData.cameraPosition = mainCamera->GetTransform()->GetPosition();
			psData.lightCount = lightCount;

			memcpy(psData.lights, &lights[0], sizeof(Light) * MAX_LIGHT_COUNT);
			// Send this to a chunk of the constant buffer heap
			// and grab the GPU handle for it so we can set it for this draw
			D3D12_GPU_DESCRIPTOR_HANDLE cbHandlePS =
				DX12Helper::GetInstance().FillNextConstantBufferAndGetGPUDescriptorHandle(
					(void*)(&psData), sizeof(PixelShaderExternalData));
			// Set this constant buffer handle
			// Note: This assumes that descriptor table 1 is the
			// place to put this particular descriptor. This
			// is based on how we set up our root signature.
			commandList->SetGraphicsRootDescriptorTable(1, cbHandlePS);

			e->Draw(commandList);
		}
		
	}

	// Present
	{
		// Transition back to present
		D3D12_RESOURCE_BARRIER rb = {};
		rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		rb.Transition.pResource = currentBackBuffer.Get();
		rb.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		rb.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		commandList->ResourceBarrier(1, &rb);

		// Must occur BEFORE present
		DX12Helper::GetInstance().CloseExecuteAndResetCommandList();

		// Present the current back buffer
		bool vsyncNecessary = vsync || !deviceSupportsTearing || isFullscreen;
		swapChain->Present(
			vsyncNecessary ? 1 : 0,
			vsyncNecessary ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Figure out which buffer is next
		currentSwapBuffer++;
		if (currentSwapBuffer >= numBackBuffers)
			currentSwapBuffer = 0;
	}

}