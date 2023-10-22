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
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();

	materialList.push_back(make_shared<Material>(Material(XMFLOAT3(1.0f, 1.0f, 1.0f), defaultPixelShader, vertexShader)));
	materialList.push_back(make_shared<Material>(Material(XMFLOAT3(1.0f, 0.0f, 0.25f), customPixelShader, vertexShader)));
	//materialList.push_back(make_shared<Material>(Material(XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), defaultPixelShader, vertexShader)));
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
	// Pick a style (uncomment one of these 3)
	ImGui::StyleColorsDark();

	//get size of constant buffer as a multiple of 16
	UINT size = sizeof(VertexShaderExternalData);
	size = (size + 15) / 16 * 16;

	D3D11_BUFFER_DESC cbDesc = {}; 
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = size; 
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;

	
	cameraList.push_back(make_shared<Camera>(Camera(((float)this->windowWidth / this->windowHeight), XMFLOAT3(0.0f, 0.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 45, 0.1f, 500.0f, 0.1f, 5.0f)));
	cameraList.push_back(make_shared<Camera>(Camera(((float)this->windowWidth / this->windowHeight), XMFLOAT3(5.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 20.0f, 0.0f), 45, 0.1f, 500.0f, 0.1f, 5.0f)));
	cameraList.push_back(make_shared<Camera>(Camera(((float)this->windowWidth / this->windowHeight), XMFLOAT3(0.0f, 1.0f, -0.5f), XMFLOAT3(0.0f, 20.0f, 0.0f), 10, 0.1f, 500.0f, 0.1f, 5.0f)));
	currentCamera = cameraList[mainCameraIndex];


	directionalLight1.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight1.direction = XMFLOAT3(1.0f, 0.0f, 0.0f);
	directionalLight1.color = XMFLOAT3(1.0f, 0.0f, 0.2f);
	directionalLight1.intensity = 1.0f;

	directionalLight2.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight2.direction = XMFLOAT3(-1.0f, 0.0f, 0.0f);
	directionalLight2.color = XMFLOAT3(0.0f, 1.0f, 0.0f);
	directionalLight2.intensity = 0.0f;

	directionalLight3.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight3.direction = XMFLOAT3(0.0f, 1.0f, 0.0f);
	directionalLight3.color = XMFLOAT3(0.0f, 1.0f, 0.0f);
	directionalLight3.intensity = 1.0f;

	pointLight1.type = LIGHT_TYPE_POINT;
	pointLight1.position = XMFLOAT3(0.0f, -1.0f, 0.0f);
	pointLight1.color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	pointLight1.intensity = 1.0f;
	pointLight1.range = 5.0f;

	pointLight2.type = LIGHT_TYPE_POINT;
	pointLight2.position = XMFLOAT3(0.0f, 0.0f, 1.0f);
	pointLight2.color = XMFLOAT3(1.0f, 1.0f, 0.0f);
	pointLight2.intensity = 1.0f;
	pointLight2.range = 10.0f;

	lights.push_back(directionalLight1);
	lights.push_back(directionalLight2);
	lights.push_back(directionalLight3);
	lights.push_back(pointLight1);
	lights.push_back(pointLight2);
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
	defaultPixelShader = std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"PixelShader.cso").c_str());
	customPixelShader = std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"CustomPS.cso").c_str());
}



// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in CPU memory
	//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	Vertex triangleVertices[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
	};

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...

	unsigned int triangleIndices[] = { 0, 1, 2 };

	std::shared_ptr<Mesh> torus = std::make_shared<Mesh>(Mesh(FixPath(L"../../Assets/Models/torus.obj").c_str(), context, device));
	

	Vertex quadVerticies[] =
	{

		{XMFLOAT3(0.2f, 0.7f, +0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{XMFLOAT3(0.2f, 0.6f, +0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0) },
		{XMFLOAT3(0.0f, +0.7f, +0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0) },
		{XMFLOAT3(0.0f, +0.8f, +0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0) },

	};
	UINT quadIndices[] = { 0,1,2,0,2,3 };
	std::shared_ptr<Mesh> cube = std::make_shared<Mesh>(Mesh(FixPath(L"../../Assets/Models/cube.obj").c_str(), context, device));
	

	Vertex miscShapeVertices[] = {
		{XMFLOAT3(-0.2f, +0.7f, 0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{XMFLOAT3(-0.4f, +0.7f, 0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{XMFLOAT3(-0.3f, +0.8f, 0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		
		{XMFLOAT3(-0.4f, +0.4f, 0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{XMFLOAT3(-0.5f, +0.55f, 0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		
		{XMFLOAT3(-0.3f, +0.3f, 0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{XMFLOAT3(-0.2f, +0.4f, 0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
	};
	UINT miscIndices[] = { 0,1,2,1,3,4,5,3,6};
	std::shared_ptr<Mesh> sphere = std::make_shared<Mesh>(Mesh(FixPath(L"../../Assets/Models/sphere.obj").c_str(), context, device));
	entityList.push_back(std::make_shared<Entity>(Entity(torus, materialList[0])));
	entityList[0]->GetTransform()->SetPosition(0.5f, 3.5f, 0.5f);
	
	entityList.push_back(std::make_shared<Entity>(Entity(torus, materialList[0])));
	entityList[1]->GetTransform()->SetRotation(0.0f, 0.0f, 0.5f);
	entityList[1]->GetTransform()->SetPosition(-2.0f, 0.1f, 0.0f);

	entityList.push_back(std::make_shared<Entity>(Entity(sphere, materialList[0])));
	entityList[2]->GetTransform()->SetScale(2.0f, 1.0f, 0.5f);
	entityList[2]->GetTransform()->SetPosition(3.0f, 0.5f, 0.0f);
	
	
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
	for (shared_ptr<Camera> camera : cameraList) {
		camera->UpdateProjectionMatrix((float)this->windowWidth / this->windowHeight);
	}

}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
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
	// Show the demo window
	//ImGui::ShowDemoWindow();

	ImGui::Begin("Assignment 7");
	ImGui::Text("Framerate %f", ImGui::GetIO().Framerate);
	ImGui::Text("Width: %i", windowWidth);
	ImGui::Text("Height: %i", windowHeight);

	if (ImGui::Button("Next Camera")) {
		mainCameraIndex = (mainCameraIndex + 1) % cameraList.size();
		currentCamera = cameraList[mainCameraIndex];
	}
	ImGui::Text("Camera Position %f %f %f", currentCamera->GetTransform()->GetPosition().x, currentCamera->GetTransform()->GetPosition().y, currentCamera->GetTransform()->GetPosition().z);
	ImGui::Text("Camera fov: %f", currentCamera->GetFov());
	ImGui::Text("Camera speed %f", currentCamera->GetMoveSpeed());
	
	for (int i = 0; i < lights.size(); i++) {
		ImGui::PushID(i);
		ImGui::Text("Light %i", i);
		ImGui::DragFloat("Intensity", &lights[i].intensity);
		ImGui::ColorEdit3("Color", &lights[i].color.x);
		ImGui::PopID();
	}

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

	//Update Geometery
	/*
	entityList[0]->GetTransform()->Rotate(0.0f, 0.0f, 0.0001f);
	entityList[0]->GetTransform()->MoveRelative(0.0001f, 0.0f, 0.0f);
	entityList[2]->GetTransform()->MoveAbsolute((float)cos(totalTime), 0.0f, 0.0f);
	entityList[3]->GetTransform()->MoveAbsolute(0.0f, -0.0001f, 0.0f);
	entityList[3]->GetTransform()->Scale(1.0001f, 1.00002f, 1.0f);
	*/
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
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erases what's on the screen)
		const float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f }; // Cornflower Blue
		context->ClearRenderTargetView(backBufferRTV.Get(), bgColor);

		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	
	// DRAW geometry
	// - These steps are generally repeated for EACH object you draw
	// - Other Direct3D calls will also be necessary to do more complex things
	for (int i = 0; i < entityList.size(); i++) {
		entityList[i]->GetMaterial()->GetPixelShader()->SetFloat3("ambient", ambientColor);
		entityList[i]->GetMaterial()->GetPixelShader()->SetData("lights", &lights[0], sizeof(Light) * (int)lights.size());
		entityList[i]->Draw(colorTint, context, currentCamera);
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
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
