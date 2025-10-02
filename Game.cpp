#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Mesh.h"
#include <memory>
#include "BufferStructs.h"

#include <DirectXMath.h>


#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// Math explanation:
// ex. number = 28
// 28 + 15 = 43
// 43 / 16  = 2.6875 (truncates to 2 bc of integer division)
// 2 * 16 = 32 (multiple of 16 that fits in 28 bytes of data)
#define MULTIPLEOF16(number) (number + 15) / 16 * 16 

// For the DirectX Math library
using namespace DirectX;

// Global direction variable for entities to use for now
float direction = 1;

// --------------------------------------------------------
// The constructor is called after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
Game::Game()
{

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());

	// Pick a style (uncomment one of these 3)
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateGeometry();


	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		Graphics::Context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);
	}

	// First calculate the size of the external data as a multiple of 16
	unsigned int sizeOfExternalData = sizeof(VertexShaderExternalData);
	sizeOfExternalData = MULTIPLEOF16(sizeOfExternalData);

	// Describe the constant buffer
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // Binds it to the pipeline as constant buffer
	cbDesc.ByteWidth = sizeOfExternalData; // size of external data as a multiple of 16
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // C++ will be writing data to the buffer
	cbDesc.Usage = D3D11_USAGE_DYNAMIC; // is a dynamic buffer because we are changing data 

	// Create the Buffer
	Graphics::Device->CreateBuffer(&cbDesc, 0, constantBuffer.GetAddressOf());

	// Bind the Constant Buffer to the Vertex Shader
	Graphics::Context->VSSetConstantBuffers(
		0, // which register to bind the buffer to 
		1, // How many buffers
		constantBuffer.GetAddressOf() // the address of buffers (if there is multiple, get the array)
	);

	globalVsData.colorTint = XMFLOAT4(1.0, 1.0, 1.0, 1.0f); // no tint, should be updated by UI

	// Create the camera
	camera = std::make_shared<Camera>(
		XMFLOAT3(0.0f, 0.0f, -5.0f), // position (not origin)
		5.0f, // Camera Speed
		0.002f, // Look Speed,
		XM_PIDIV4, // FOV (In Radians)
		Window::AspectRatio(), // Aspect Ratio
		0.01f, // Near Clip Distance
		100.0f, // Far Clip Distance
		ProjectionType::PERSPECTIVE // Which Projection Type?
	);

}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
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
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		Graphics::Device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		Graphics::Device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
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
	Vertex vertices[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	};

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int indices[] = { 0, 1, 2 };


	// Vertices of the letter J shape
	Vertex verticesJ[] = {
		{ XMFLOAT3(-0.9f, +0.9f, +0.0f), red}, //0
		{ XMFLOAT3(-0.9f, +0.8f, +0.0f), red}, //1
		{ XMFLOAT3(-0.75f, +0.9f, +0.0f), red}, //2
		{ XMFLOAT3(-0.75f, +0.8f, +0.0f), red}, //3
		{ XMFLOAT3(-0.65f, +0.9f, +0.0f), }, //4
		{ XMFLOAT3(-0.65f, +0.8f, +0.0f), }, //5
		{ XMFLOAT3(-0.55f, +0.9f, +0.0f), }, //6
		{ XMFLOAT3(-0.55f, +0.8f, +0.0f), }, //7
		{ XMFLOAT3(-0.65f, +0.6f, +0.0f), }, //8
		{ XMFLOAT3(-0.75f, +0.7f, +0.0f), red}, //9
		{ XMFLOAT3(-0.85f, +0.6f, +0.0f), red}, //10
		{ XMFLOAT3(-0.85f, +0.7f, +0.0f), red}, //11
	};

	// Indices of the letter J shape
	unsigned int indicesJ[] = {
		1, 0, 2,
		1, 2, 3,
		3, 2, 5,
		5, 2, 4,
		5, 4, 6,
		5, 6, 7,
		9, 3, 5,
		8, 9, 5,
		10, 9, 8,
		10, 11, 9
	};


	// Vertices of the letter A Shape
	Vertex verticesA[] = {
		{ XMFLOAT3(-0.45f, +0.6f, +0.0f),  }, //0
		{ XMFLOAT3(-0.3f, +0.9f, +0.0f),  }, //1
		{ XMFLOAT3(-0.3f, +0.8f, +0.0f),  }, //2
		{ XMFLOAT3(-0.35f, +0.6f, +0.0f),  }, //3
		{ XMFLOAT3(-0.15f, +0.6f, +0.0f), red }, //4
		{ XMFLOAT3(-0.25f, +0.6f, +0.0f), red }, //5
	};

	// Indices of the letter A Shape
	unsigned int indicesA[] = {
		0, 1, 2,
		2, 3, 0,
		2, 1, 4,
		5, 2, 4
	};

	// push each mesh to the meshes array
	meshes.push_back(std::make_shared<Mesh>(vertices, indices, (unsigned int)ARRAYSIZE(vertices), (unsigned int)ARRAYSIZE(indices), "Triangle"));
	meshes.push_back(std::make_shared<Mesh>(verticesJ, indicesJ, (unsigned int)ARRAYSIZE(verticesJ), (unsigned int)ARRAYSIZE(indicesJ), "Letter J"));
	meshes.push_back(std::make_shared<Mesh>(verticesA, indicesA, (unsigned int)ARRAYSIZE(verticesA), (unsigned int)ARRAYSIZE(indicesA), "Letter A"));

	// push game entities to the entity array
	entities.push_back(std::make_shared<Entity>(meshes[0]));
	entities.push_back(std::make_shared<Entity>(meshes[1]));
	entities.push_back(std::make_shared<Entity>(meshes[2])); // Same Mesh
	entities.push_back(std::make_shared<Entity>(meshes[1])); // Same Mesh
	entities.push_back(std::make_shared<Entity>(meshes[2])); // Same Mesh
}



// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	if (camera) camera->UpdateProjectionMatrix(Window::AspectRatio());
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	RefreshUI(deltaTime);
	BuildUI();
	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	float scale = (float)sin(totalTime * 5) * 0.5f + 1.0f;


	// Triangle
	entities[0]->GetTransform()->SetScale(scale, scale, scale);
	entities[0]->GetTransform()->Rotate(0, 0, deltaTime * 1.0f);

	// J's
	if (entities[1]->GetTransform()->GetPosition().x > 1.5f && direction == 1) {
		direction *= -1;
	}

	if (entities[1]->GetTransform()->GetPosition().x < 0.0f && direction == -1) {
		direction *= -1;
	}

	entities[1]->GetTransform()->MoveAbsolute(deltaTime * .5f * direction, 0, 0);
	entities[3]->GetTransform()->MoveAbsolute(0, -deltaTime * .5f * direction, 0);

	// A's
	entities[2]->GetTransform()->Rotate(0, 0, deltaTime * .5f * direction);
	entities[4]->GetTransform()->Rotate(0, 0, -deltaTime * .5f * direction);

	camera->Update(deltaTime);

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
		// Clear the back buffer (erase what's on screen) and depth buffer
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(), color);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// For each entity
	for (auto& entity : entities) {
		// Create the Shader Data struct to fill in
		VertexShaderExternalData vsData = {};

		// set the tint to the global tint and matrix to entity World Matrix
		vsData.colorTint = globalVsData.colorTint;
		vsData.world = entity->GetTransform()->GetWorldMatrix();
		vsData.viewMatrix = camera->GetView();
		vsData.projectionMatrix = camera->GetProjection();

		// Copy the data we intend to use to the constant buffer
		D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
		Graphics::Context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);

		// copy external data struct to the mapped buffer
		memcpy(mappedBuffer.pData, &vsData, sizeof(globalVsData));

		// Unmap the buffer
		Graphics::Context->Unmap(constantBuffer.Get(), 0);

		// Draw entity
		entity->Draw();
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		ImGui::Render(); // Turns this frame’s UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen

		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}

void Game::RefreshUI(float deltaTime) {
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();

	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
}

void Game::BuildUI()
{
	// Title of Window
	ImGui::Begin("Julian's Awesome First Window :D");

	// Inspector Collapsing Header
	if (ImGui::CollapsingHeader("Inspector")) {

		// App Details
		if (ImGui::TreeNode("App Details")) {

			// FPS Display
			ImGui::Text("Framerate: %f fps", ImGui::GetIO().Framerate);

			// Window Resolution Display
			ImGui::Text("Window Client Size: %dx%d", Window::Width(), Window::Height());

			// Background Color Editor
			ImGui::ColorEdit4("RGBA color editor", color);

			// Ends this Tree
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Meshes")) {
			for (int i = 0; i < meshes.size(); i++) {

				ImGui::PushID(meshes[i].get());

				// Mesh Name and Data
				if (ImGui::TreeNode("Mesh: %s", meshes[i]->GetName())) {
					ImGui::Text("Triangles: %d", meshes[i]->CalculateTris());
					ImGui::Text("Vertices: %d", meshes[i]->GetVertexCount());
					ImGui::Text("Indices: %d", meshes[i]->GetIndexCount());
					ImGui::TreePop();
				}

				ImGui::PopID();
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Scene Entities")) {
			for (int i = 0; i < entities.size(); i++) {
				ImGui::PushID(entities[i].get());

				// Entity ID
				if (ImGui::TreeNode("Entity Node", "Entity %d", i)) {
					XMFLOAT3 pos = entities[i]->GetTransform()->GetPosition();
					XMFLOAT3 rot = entities[i]->GetTransform()->GetPitchYawRoll();
					XMFLOAT3 sca = entities[i]->GetTransform()->GetScale();

					if (ImGui::DragFloat3("Position", &pos.x, 0.01f)) entities[i]->GetTransform()->SetPosition(pos);
					if (ImGui::DragFloat3("Rotation (Radians)", &rot.x, 0.01f)) entities[i]->GetTransform()->SetRotation(rot);
					if (ImGui::DragFloat3("Scale", &sca.x, 0.01f)) entities[i]->GetTransform()->SetScale(sca);

					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
	}

	// Editor Collapsing Header
	if (ImGui::CollapsingHeader("Editor")) {
		ImGui::ColorEdit4("Color Tint", &globalVsData.colorTint.x);
	}


	ImGui::End();

}

