#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "BufferStructs.h"
// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>
#include "Entity.h"
#include <iostream>
#include <WICTextureLoader.h>
// For the DirectX Math library
using namespace DirectX;
using namespace std;
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
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Delete all objects manually created within this class
//  - Release() all Direct3D objects created within this class
// --------------------------------------------------------
Game::~Game()
{
	// Call delete or delete[] on any objects or arrays you've
	// created using new or new[] within this class
	// - Note: this is unnecessary if using smart pointers

	// Call Release() on any Direct3D objects made within this class
	// - Note: this is unnecessary for D3D objects stored in ComPtrs
	
	//shutdown debug UI system
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// --------------------------------------------------------
// Called once per program, after Direct3D and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	LoadShaders();
	LoadMaterials();
	CreateGeometry();
	
	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
	}

	//initialize ImGui system + backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());
	ImGui::StyleColorsDark();

	//get size of constant buffer as a multiple of 16
	UINT size = sizeof(VertexShaderExternalData);
	size = (size + 15) / 16 * 16;

	D3D11_BUFFER_DESC cbDesc = {}; 
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = size; 
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;

	CreateCameras();
	CreateLights();
	InitShadowMapResources();
	InitPostProcessResources();
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = std::make_shared<SimpleVertexShader>(device, context,
		FixPath(L"VertexShader.cso").c_str());
	normalMapVertexShader = std::make_shared<SimpleVertexShader>(device, context,
		FixPath(L"VertexShader_NormalMap.cso").c_str());
	defaultPixelShader = std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"PixelShader.cso").c_str());
	customPixelShader = std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"CustomPS.cso").c_str());
	normalMapPixelShader = std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"PixelShader_NormalMap.cso").c_str());
	skyVertexShader = std::make_shared<SimpleVertexShader>(device, context,
		FixPath(L"SkyVertexShader.cso").c_str());
	skyPixelShader = std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"SkyPixelShader.cso").c_str());
	shadowVertexShader = std::make_shared<SimpleVertexShader>(device, context,
		FixPath(L"ShadowVertexShader.cso").c_str());
	celPixelShader = std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"CelPixelShader.cso").c_str());
	ppVS = std::make_shared<SimpleVertexShader>(device, context,
		FixPath(L"FullScreenTriangleVertexShader.cso").c_str());
	ppPS = std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"BlurPixelShader.cso").c_str());
}

void Game::LoadMaterials()
{
	Microsoft::WRL::ComPtr<ID3D11SamplerState> clampSampler;

	D3D11_SAMPLER_DESC samplerDescription = {};
	samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&samplerDescription, samplerState.GetAddressOf());

	//clamp sampler for cel shader
	samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	device->CreateSamplerState(&samplerDescription, clampSampler.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> marbleTextureResouce;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> brickTextureResouce;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleStoneAlbedoResouce;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleStoneNormalResouce;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleStoneMetalResouce;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleStoneRoughResouce;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorAlbedoResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorNormalResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorMetalResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorRoughResource;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeAlbedoResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeNormalResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeMetalResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeRoughResource;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintAlbedoResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintNormalResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintMetalResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintRoughResource;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodAlbedoResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodNormalResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodMetalResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodRoughResource;

	//cel shading resources
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rampTexture2Band;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rampTexture3Band;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rampTexture4Band;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> flatNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> redAlbedo;

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/rock.png").c_str(), nullptr, marbleTextureResouce.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/brick.png").c_str(), nullptr, brickTextureResouce.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/cobblestone_albedo.png").c_str(), nullptr, cobbleStoneAlbedoResouce.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/cobblestone_normals.png").c_str(), nullptr, cobbleStoneNormalResouce.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/cobblestone_metal.png").c_str(), nullptr, cobbleStoneMetalResouce.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/cobblestone_roughness.png").c_str(), nullptr, cobbleStoneRoughResouce.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/floor_albedo.png").c_str(), nullptr, floorAlbedoResource.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/floor_normals.png").c_str(), nullptr, floorNormalResource.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/floor_metal.png").c_str(), nullptr, floorMetalResource.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/floor_roughness.png").c_str(), nullptr, floorRoughResource.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/bronze_albedo.png").c_str(), nullptr, bronzeAlbedoResource.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/bronze_normals.png").c_str(), nullptr, bronzeNormalResource.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/bronze_metal.png").c_str(), nullptr, bronzeMetalResource.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/bronze_roughness.png").c_str(), nullptr, bronzeRoughResource.GetAddressOf());


	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/paint_albedo.png").c_str(), nullptr, paintAlbedoResource.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/paint_normals.png").c_str(), nullptr, paintNormalResource.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/paint_metal.png").c_str(), nullptr, paintMetalResource.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/paint_roughness.png").c_str(), nullptr, paintRoughResource.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/wood_albedo.png").c_str(), nullptr, woodAlbedoResource.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/wood_normals.png").c_str(), nullptr, woodNormalResource.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/wood_metal.png").c_str(), nullptr, woodMetalResource.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/wood_roughness.png").c_str(), nullptr, woodRoughResource.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/rampTexture2Band.png").c_str(), nullptr, rampTexture2Band.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/rampTexture3Band.png").c_str(), nullptr, rampTexture3Band.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/rampTexture4Band.png").c_str(), nullptr, rampTexture4Band.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/flat_normals.png").c_str(), nullptr, flatNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/red.png").c_str(), nullptr, redAlbedo.GetAddressOf());

	materialList.push_back(make_shared<Material>(Material(XMFLOAT3(1.0f, 1.0f, 1.0f), defaultPixelShader, vertexShader)));
	materialList.push_back(make_shared<Material>(Material(XMFLOAT3(1.0f, 1.0f, 1.0f), defaultPixelShader, vertexShader)));

	//use no normal map
	materialList[0]->AddTextureSRV("Albedo", floorAlbedoResource);
	materialList[0]->AddTextureSRV("MetalnessMap", floorMetalResource);
	materialList[0]->AddTextureSRV("RoughnessMap", floorRoughResource);
	materialList[0]->AddSample("SamplerOptions", samplerState);

	materialList[1]->SetTetxureOffset(0.1f);
	materialList[1]->SetTextureScale(5);
	materialList[1]->AddTextureSRV("Albedo", cobbleStoneAlbedoResouce);
	materialList[1]->AddTextureSRV("MetalnessMap", cobbleStoneMetalResouce);
	materialList[1]->AddTextureSRV("RoughnessMap", cobbleStoneRoughResouce);
	materialList[1]->AddSample("SamplerOptions", samplerState);

	//are normal mapped
	materialList.push_back(make_shared<Material>(Material(XMFLOAT3(1.0f, 1.0f, 1.0f), normalMapPixelShader, normalMapVertexShader)));
	materialList[2]->AddTextureSRV("Albedo", cobbleStoneAlbedoResouce);
	materialList[2]->AddTextureSRV("NormalMap", cobbleStoneNormalResouce);
	materialList[2]->AddTextureSRV("MetalnessMap", cobbleStoneMetalResouce);
	materialList[2]->AddTextureSRV("RoughnessMap", cobbleStoneRoughResouce);
	materialList[2]->AddSample("SamplerOptions", samplerState);

	materialList.push_back(make_shared<Material>(Material(XMFLOAT3(1.0f, 1.0f, 1.0f), normalMapPixelShader, normalMapVertexShader)));
	materialList[3]->AddTextureSRV("Albedo", floorAlbedoResource);
	materialList[3]->AddTextureSRV("NormalMap", floorNormalResource);
	materialList[3]->AddTextureSRV("MetalnessMap", floorMetalResource);
	materialList[3]->AddTextureSRV("RoughnessMap", floorRoughResource);
	materialList[3]->AddSample("SamplerOptions", samplerState);

	materialList.push_back(make_shared<Material>(Material(XMFLOAT3(1.0f, 1.0f, 1.0f), normalMapPixelShader, normalMapVertexShader)));
	materialList[4]->AddTextureSRV("Albedo", bronzeAlbedoResource);
	materialList[4]->AddTextureSRV("NormalMap", bronzeNormalResource);
	materialList[4]->AddTextureSRV("MetalnessMap", bronzeMetalResource);
	materialList[4]->AddTextureSRV("RoughnessMap", bronzeRoughResource);
	materialList[4]->AddSample("SamplerOptions", samplerState);

	materialList.push_back(make_shared<Material>(Material(XMFLOAT3(1.0f, 1.0f, 1.0f), normalMapPixelShader, normalMapVertexShader)));
	materialList[5]->AddTextureSRV("Albedo", paintAlbedoResource);
	materialList[5]->AddTextureSRV("NormalMap", paintNormalResource);
	materialList[5]->AddTextureSRV("MetalnessMap", paintMetalResource);
	materialList[5]->AddTextureSRV("RoughnessMap", paintRoughResource);
	materialList[5]->AddSample("SamplerOptions", samplerState);


	materialList.push_back(make_shared<Material>(Material(XMFLOAT3(1.0f, 1.0f, 1.0f), normalMapPixelShader, normalMapVertexShader)));
	materialList[6]->AddTextureSRV("Albedo", woodAlbedoResource);
	materialList[6]->AddTextureSRV("NormalMap", woodNormalResource);
	materialList[6]->AddTextureSRV("MetalnessMap", woodMetalResource);
	materialList[6]->AddTextureSRV("RoughnessMap", woodRoughResource);
	materialList[6]->AddSample("SamplerOptions", samplerState);

	materialList.push_back(make_shared<Material>(Material(XMFLOAT3(1.0f, 1.0f, 1.0f), celPixelShader, normalMapVertexShader)));
	materialList[7]->AddTextureSRV("Albedo", woodAlbedoResource);
	materialList[7]->AddTextureSRV("NormalMap", woodNormalResource);
	materialList[7]->AddTextureSRV("RampTexture", rampTexture3Band);
	materialList[7]->AddSample("SamplerOptions", samplerState);
	materialList[7]->AddSample("ClampSampler", clampSampler);

	materialList.push_back(make_shared<Material>(Material(XMFLOAT3(1.0f, 1.0f, 1.0f), celPixelShader, normalMapVertexShader)));
	materialList[8]->AddTextureSRV("Albedo", bronzeAlbedoResource);
	materialList[8]->AddTextureSRV("NormalMap", bronzeNormalResource);
	materialList[8]->AddTextureSRV("RampTexture", rampTexture2Band);
	materialList[8]->AddSample("SamplerOptions", samplerState);
	materialList[8]->AddSample("ClampSampler", clampSampler);
	materialList[8]->SetRoughness(1.0f);

	materialList.push_back(make_shared<Material>(Material(XMFLOAT3(1.0f, 1.0f, 1.0f), celPixelShader, normalMapVertexShader)));
	materialList[9]->AddTextureSRV("Albedo", bronzeAlbedoResource);
	materialList[9]->AddTextureSRV("NormalMap", flatNormal);
	materialList[9]->AddTextureSRV("RampTexture", rampTexture2Band);
	materialList[9]->AddSample("SamplerOptions", samplerState);
	materialList[9]->AddSample("ClampSampler", clampSampler);
	materialList[9]->SetRoughness(0.9f);

	materialList.push_back(make_shared<Material>(Material(XMFLOAT3(1.0f, 1.0f, 1.0f), celPixelShader, normalMapVertexShader)));
	materialList[10]->AddTextureSRV("Albedo", redAlbedo);
	materialList[10]->AddTextureSRV("NormalMap", flatNormal);
	materialList[10]->AddTextureSRV("RampTexture", rampTexture4Band);
	materialList[10]->AddSample("SamplerOptions", samplerState);
	materialList[10]->AddSample("ClampSampler", clampSampler);

}

void Game::CreateGeometry()
{
	std::shared_ptr<Mesh> torus = std::make_shared<Mesh>(Mesh(FixPath(L"../../Assets/Models/torus.obj").c_str(), context, device));
	std::shared_ptr<Mesh> cube = std::make_shared<Mesh>(Mesh(FixPath(L"../../Assets/Models/cube.obj").c_str(), context, device));
	std::shared_ptr<Mesh> sphere = std::make_shared<Mesh>(Mesh(FixPath(L"../../Assets/Models/sphere.obj").c_str(), context, device));

	entityList.push_back(std::make_shared<Entity>(Entity(cube, materialList[2])));
	entityList[0]->GetTransform()->SetPosition(0.5f, 3.5f, 1.5f);
	
	entityList.push_back(std::make_shared<Entity>(Entity(torus, materialList[5])));
	entityList[1]->GetTransform()->SetRotation(0.0f, 0.0f, 0.5f);
	entityList[1]->GetTransform()->SetPosition(-2.0f, 0.1f, 0.0f);

	entityList.push_back(std::make_shared<Entity>(Entity(sphere, materialList[4])));
	entityList[2]->GetTransform()->SetPosition(3.0f, 0.5f, 0.0f);

	entityList.push_back(std::make_shared<Entity>(Entity(sphere, materialList[3])));
	entityList[3]->GetTransform()->SetPosition(7.0f, 0.5f, 0.0f);
	
	entityList.push_back(std::make_shared<Entity>(Entity(sphere, materialList[8])));
	entityList[4]->GetTransform()->SetPosition(-7.0f, 0.5f, 0.0f);

	entityList.push_back(std::make_shared<Entity>(Entity(sphere, materialList[9])));
	entityList[5]->GetTransform()->SetPosition(-7.0f, 3.5f, 0.0f);

	entityList.push_back(std::make_shared<Entity>(Entity(sphere, materialList[10])));
	entityList[6]->GetTransform()->SetPosition(-10.0f, 3.5f, 0.0f);
	
	//floor
	entityList.push_back(std::make_shared<Entity>(Entity(cube, materialList[6])));
	entityList[7]->GetTransform()->SetPosition(0.0f, -3.5f, 0.0f);
	entityList[7]->GetTransform()->SetScale(20.0f, 1.0f, 20.0f);

	skyBox = make_shared<Sky>(Sky(cube, samplerState, device, context, skyPixelShader, skyVertexShader, FixPath(L"../../Assets/Textures/Sky/right.png").c_str(), FixPath(L"../../Assets/Textures/Sky/left.png").c_str(),
		FixPath(L"../../Assets/Textures/Sky/up.png").c_str(), FixPath(L"../../Assets/Textures/Sky/down.png").c_str(), FixPath(L"../../Assets/Textures/Sky/front.png").c_str(), FixPath(L"../../Assets/Textures/Sky/back.png").c_str()));
}

void Game::CreateLights()
{
	sun.type = LIGHT_TYPE_DIRECTIONAL;
	sun.direction = XMFLOAT3(1.0f, -0.25f, 0.0f);
	sun.color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	sun.intensity = 0.5f;

	directionalLight2.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight2.direction = XMFLOAT3(-1.0f, 0.0f, 0.0f);
	directionalLight2.color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	directionalLight2.intensity = 0.0f;

	directionalLight3.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight3.direction = XMFLOAT3(0.0f, 1.0f, 0.0f);
	directionalLight3.color = XMFLOAT3(0.0f, 1.0f, 0.0f);
	directionalLight3.intensity = 0.0f;

	pointLight1.type = LIGHT_TYPE_POINT;
	pointLight1.position = XMFLOAT3(-6.0f, 0.0f, 0.0f);
	pointLight1.color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	pointLight1.intensity = 0.0f;
	pointLight1.range = 5.0f;

	pointLight2.type = LIGHT_TYPE_POINT;
	pointLight2.position = XMFLOAT3(0.0f, 0.0f, 1.0f);
	pointLight2.color = XMFLOAT3(1.0f, 1.0f, 0.0f);
	pointLight2.intensity = 0.0f;
	pointLight2.range = 10.0f;

	lights.push_back(sun);
	lights.push_back(directionalLight2);
	lights.push_back(directionalLight3);
	lights.push_back(pointLight1);
	lights.push_back(pointLight2);
}

void Game::CreateCameras()
{
	cameraList.push_back(make_shared<Camera>(Camera(((float)this->windowWidth / this->windowHeight), XMFLOAT3(0.0f, 0.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 45, 0.1f, 500.0f, 0.01f, 5.0f)));
	cameraList.push_back(make_shared<Camera>(Camera(((float)this->windowWidth / this->windowHeight), XMFLOAT3(5.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 20.0f, 0.0f), 45, 0.1f, 500.0f, 0.1f, 5.0f)));
	cameraList.push_back(make_shared<Camera>(Camera(((float)this->windowWidth / this->windowHeight), XMFLOAT3(0.0f, 1.0f, -0.5f), XMFLOAT3(0.0f, 20.0f, 0.0f), 10, 0.1f, 500.0f, 0.1f, 5.0f)));
	currentCamera = cameraList[mainCameraIndex];
}

void Game::InitShadowMapResources()
{
	// Create the actual texture that will be the shadow map
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	//map should be square with sizes as a power of 2
	shadowDesc.Width = shadowMapResolution; 
	shadowDesc.Height = shadowMapResolution; 
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	// Create the depth/stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(
		shadowTexture.Get(),
		&shadowDSDesc,
		shadowDSV.GetAddressOf());
	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(
		shadowTexture.Get(),
		&srvDesc,
		shadowSRV.GetAddressOf());

	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Min. precision units, not world units!
	shadowRastDesc.SlopeScaledDepthBias = 1.0f; // Bias more based on slope
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f; // Only need the first component
	device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	//XMMATRIX fromLightView = XMMatrixLookToLH(-XMLoadFloat3(&sun.direction) * 20, XMLoadFloat3(&sun.direction), XMVectorSet(0, 1, 0, 0));
	//XMStoreFloat4x4(&lightViewMatrix, fromLightView);
	XMMATRIX shView = XMMatrixLookAtLH(
		XMVectorSet(0, sun.direction.x * 20, sun.direction.y * 20, sun.direction.z * 20),
		XMVectorSet(0, 0, 0, 0),
		XMVectorSet(0, 1, 0, 0));
	XMStoreFloat4x4(&lightViewMatrix, shView);

	XMMATRIX fromLightProjection = XMMatrixOrthographicLH(lightProjectionSize, lightProjectionSize, 1.0f, 100.0f);
	XMStoreFloat4x4(&lightProjectionMatrix, fromLightProjection);
}

void Game::InitPostProcessResources()
{
	// Sampler state for post processing
	D3D11_SAMPLER_DESC ppSampDesc = {};
	ppSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ppSampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&ppSampDesc, ppSampler.GetAddressOf());

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = windowWidth;
	textureDesc.Height = windowHeight;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> ppTexture;
	device->CreateTexture2D(&textureDesc, 0, ppTexture.GetAddressOf());

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	device->CreateRenderTargetView(
		ppTexture.Get(),
		&rtvDesc,
		ppRTV.ReleaseAndGetAddressOf());

	// Create the default Shader Resource View by passing in 0
	device->CreateShaderResourceView(
		ppTexture.Get(),
		0,
		ppSRV.ReleaseAndGetAddressOf());
}

void Game::ResizePostProcessResources()
{
	ppRTV.Reset();
	ppSRV.Reset();

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = windowWidth;
	textureDesc.Height = windowHeight;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> ppTexture;
	device->CreateTexture2D(&textureDesc, 0, ppTexture.GetAddressOf());

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(
		ppTexture.Get(),
		&rtvDesc,
		ppRTV.ReleaseAndGetAddressOf());
	device->CreateShaderResourceView(
		ppTexture.Get(),
		0,
		ppSRV.ReleaseAndGetAddressOf());
}


// --------------------------------------------------------
// Handle resizing to match the new window size.
//  - DXCore needs to resize the back buffer
//  - Update camera projection matrix
//  -resize post process 
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();
	for (shared_ptr<Camera> camera : cameraList) {
		camera->UpdateProjectionMatrix((float)this->windowWidth / this->windowHeight);
	}

	ResizePostProcessResources();
}


void Game::Update(float deltaTime, float totalTime)
{
	// Feed fresh input data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)this->windowWidth;
	io.DisplaySize.y = (float)this->windowHeight;
	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Determine new input capture
	Input& input = Input::GetInstance();
	input.SetKeyboardCapture(io.WantCaptureKeyboard);
	input.SetMouseCapture(io.WantCaptureMouse);

	ImGui::Begin("Debug Controls");
	ImGui::Text("Framerate %f", ImGui::GetIO().Framerate);
	ImGui::Text("Width: %i", windowWidth);
	ImGui::Text("Height: %i", windowHeight);
	ImGui::Image(shadowSRV.Get(), ImVec2(512, 512));

	//post process controls
	ImGui::SliderInt("Blur Radius", &blurRadius, 0, 100);

	//camera controls
	if (ImGui::Button("Next Camera")) {
		mainCameraIndex = (mainCameraIndex + 1) % cameraList.size();
		currentCamera = cameraList[mainCameraIndex];
	}
	ImGui::Text("Camera Position %f %f %f", currentCamera->GetTransform()->GetPosition().x, currentCamera->GetTransform()->GetPosition().y, currentCamera->GetTransform()->GetPosition().z);
	ImGui::Text("Camera fov: %f", currentCamera->GetFov());
	ImGui::Text("Camera speed %f", currentCamera->GetMoveSpeed());
	
	//light controls
	for (int i = 0; i < lights.size(); i++) {
		ImGui::PushID(i);
		ImGui::Text("Light %i", i);
		ImGui::DragFloat("Intensity", &lights[i].intensity);
		ImGui::ColorEdit3("Color", &lights[i].color.x);
		ImGui::PopID();
	}

	//entity controls
	for (int i = 0; i < entityList.size(); i++) {
		ImGui::PushID(i);
		ImGui::Text("Entity %i", i);
		XMFLOAT3 position = entityList[i]->GetTransform()->GetPosition();
		
		if (ImGui::DragFloat3("Position", &position.x)) {
			entityList[i]->GetTransform()->SetPosition(position);
		}
		
		XMFLOAT3 rotation = entityList[i]->GetTransform()->GetPitchYawRoll();
		if (ImGui::DragFloat3("Rotation", &rotation.x)) {
			entityList[i]->GetTransform()->SetRotation(rotation);
		}
		
		XMFLOAT3 scale = entityList[i]->GetTransform()->GetScale();
		if (ImGui::DragFloat3("Scale", &scale.x)) {
			entityList[i]->GetTransform()->SetScale(scale);
		}
		ImGui::PopID();
	}
	

	ImGui::End();
	currentCamera->Update(deltaTime);

	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	{
		// Clear the back buffer (erases what's on the screen)
		const float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f }; // Cornflower Blue
		context->ClearRenderTargetView(backBufferRTV.Get(), bgColor);

		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		context->ClearRenderTargetView(ppRTV.Get(), bgColor);
	}
	

	DrawShadowMap();

	//set render target
	context->OMSetRenderTargets(1, ppRTV.GetAddressOf(), depthBufferDSV.Get());

	// DRAW geometry
	for (int i = 0; i < entityList.size(); i++) {
		auto ps = entityList[i]->GetMaterial()->GetPixelShader();
		auto vs = entityList[i]->GetMaterial()->GetVertexShader();

		ps->SetFloat3("ambient", ambientColor);
		ps->SetData("lights", &lights[0], sizeof(Light) * (int)lights.size());
		ps->SetShaderResourceView("ShadowMap", shadowSRV);
		ps->SetSamplerState("ShadowSampler", shadowSampler);
		vs->SetMatrix4x4("lightView", lightViewMatrix);
		vs->SetMatrix4x4("lightProjection", lightProjectionMatrix);
		entityList[i]->Draw(colorTint, context, currentCamera);
	}

	skyBox->Draw(context, currentCamera);

	// Frame END
	
	//unbind all SRVs
	ID3D11ShaderResourceView* nullSRVs[128] = {};
	context->PSSetShaderResources(0, 128, nullSRVs);

	//now that geometery has been rendered to a stored target set render to target to recieve post process
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), 0);

	// Activate shaders and bind resources
	ppVS->SetShader();
	ppPS->SetShader();
	ppPS->SetShaderResourceView("Pixels", ppSRV.Get());
	ppPS->SetSamplerState("ClampSampler", ppSampler.Get());
	ppPS->SetInt("blurRadius", blurRadius);
	ppPS->SetFloat("pixelWidth", 1.0f / windowWidth);
	ppPS->SetFloat("pixelHeight", 1.0f / windowHeight);
	ppPS->CopyAllBufferData();

	context->Draw(3, 0); // Draw exactly 3 vertices (one triangle) for post process

	{
		// Present the back buffer to the user
		//  - Puts the results of what we've drawn onto the window
		//  - Without this, the user never sees anything
		bool vsyncNecessary = vsync || !deviceSupportsTearing || isFullscreen;

		//draw debug UI
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		swapChain->Present(
			vsyncNecessary ? 1 : 0,
			vsyncNecessary ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Must re-bind buffers after presenting, as they become unbound
		context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	}
}

void Game::DrawShadowMap()
{
	context->OMSetRenderTargets(0, 0, shadowDSV.Get());
	context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	context->RSSetState(shadowRasterizer.Get());

	//turn off the pixel shader
	context->PSSetShader(0, 0, 0);

	D3D11_VIEWPORT viewport = {};
	viewport.Width = (float)shadowMapResolution;
	viewport.Height = (float)shadowMapResolution;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	shadowVertexShader->SetShader();
	shadowVertexShader->SetMatrix4x4("view", lightViewMatrix);
	shadowVertexShader->SetMatrix4x4("projection", lightProjectionMatrix);
	// Loop and draw all entities
	for (auto& e : entityList)
	{
		shadowVertexShader->SetMatrix4x4("world", e->GetTransform()->GetWorldMatrix());
		shadowVertexShader->CopyAllBufferData();
		// Draw the mesh directly to avoid the entity's material
		e->GetMesh()->Draw();
	}

	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	viewport.Width = (float)this->windowWidth;
	viewport.Height = (float)this->windowHeight;
	context->RSSetViewports(1, &viewport);
	context->RSSetState(0);
}
