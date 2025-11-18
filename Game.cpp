#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Mesh.h"
#include <memory>
#include "BufferStructs.h"
#include "Material.h"

#include <DirectXMath.h>
#include "WICTextureLoader.h"


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

	// Pick a style
	ImGui::StyleColorsDark();


	// LoadAssets and Entities
	LoadAssetsAndCreateEntities();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3DBlob* vertexShaderBlob;
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);
		// Create an input layout 
		//  - This describes the layout of data sent to a vertex shader
		//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
		//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
		//  - Luckily, we already have that loaded (the vertex shader blob above)
		{
			D3D11_INPUT_ELEMENT_DESC inputElements[4] = {};

			// Set up the first element - a position, which is 3 float values
			inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
			inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
			inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

			// Set up the second element - a UV Texcoord, which is 2 more float values
			inputElements[1].Format = DXGI_FORMAT_R32G32_FLOAT;					// 2x 32-bit floats
			inputElements[1].SemanticName = "TEXCOORD";							// Match our vertex shader input!
			inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

			// Set up the third element - a Normal, which is 3 more float values
			inputElements[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;			    // 3x 32-bit floats
			inputElements[2].SemanticName = "NORMAL";							// Match vertex shader input!
			inputElements[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;  // After previous element

			// Set up the fourth element - a tangent, which is 3 more float values
			inputElements[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			inputElements[3].SemanticName = "TANGENT";
			inputElements[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			

			// Create the input layout, verifying our description against actual shader code
			Graphics::Device->CreateInputLayout(
				inputElements,							// An array of descriptions
				4,										// How many elements in that array?
				vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
				vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
				inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
		}

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		Graphics::Context->IASetInputLayout(inputLayout.Get());
	}

	// Create the cameras
	std::shared_ptr camera1 = std::make_shared<Camera>(
		XMFLOAT3(0.0f, 5.0f, -20.0f), // position (not origin)
		5.0f, // Camera Speed
		0.002f, // Look Speed,
		XM_PIDIV4, // FOV (In Radians)
		Window::AspectRatio(), // Aspect Ratio
		0.01f, // Near Clip Distance
		100.0f, // Far Clip Distance
		ProjectionType::PERSPECTIVE // Which Projection Type?
	);

	std::shared_ptr camera2 = std::make_shared<Camera>(
		XMFLOAT3(-5.0f, 5.0f, -10.0f), // position (not origin)
		5.0f, // Camera Speed
		0.002f, // Look Speed,
		XM_PIDIV2, // FOV (In Radians)
		Window::AspectRatio(), // Aspect Ratio
		0.01f, // Near Clip Distance
		100.0f, // Far Clip Distance
		ProjectionType::PERSPECTIVE // Which Projection Type?
	);

	std::shared_ptr camera3 = std::make_shared<Camera>(
		XMFLOAT3(5.0f, 0.0f, -5.0f), // position (not origin)
		5.0f, // Camera Speed
		0.002f, // Look Speed,
		XM_PI/6, // FOV (In Radians)
		Window::AspectRatio(), // Aspect Ratio
		0.01f, // Near Clip Distance
		100.0f, // Far Clip Distance
		ProjectionType::ORTHOGRAPHIC // Which Projection Type?,
	);

	camera3->SetOrthoGraphicWidth(25);

	cameras.push_back(camera1);
	cameras.push_back(camera2);
	cameras.push_back(camera3);

	activeCameraIndex = 0;

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



void Game::LoadAssetsAndCreateEntities()
{
	// create sampler
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // What happens outside the 0-1 uv range?
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;		// How do we handle sampling "between" pixels?
	sampDesc.MaxAnisotropy = 16;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Graphics::Device->CreateSamplerState(&sampDesc, samplerState.GetAddressOf());

	// load textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> tilesSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> crackSRV;

	// Wood
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), 
		FixPath(L"../../Assets/Textures/wood_planks.png").c_str(), 0, woodSRV.GetAddressOf());

	// Tiles
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), 
		FixPath(L"../../Assets/Textures/checkered_pavement_tiles.png").c_str(), 0, tilesSRV.GetAddressOf());

	// Crack
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/grayscale_crack.png").c_str(), 0, crackSRV.GetAddressOf());

	// load shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader> basicVShader = LoadVertexShader(L"VertexShader.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> basicPShader = LoadPixelShader(L"PixelShader.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> fancyPixelShader = LoadPixelShader(L"CustomPS.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> normalPreviewPS = LoadPixelShader(L"DebugNormalsPS.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> uvPreviewPS = LoadPixelShader(L"DebugUVsPS.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> twoTexPShader = LoadPixelShader(L"TwoTexturePS.cso");


	// create materials from shaders
	std::shared_ptr<Material> woodMat = std::make_shared<Material>("Wood", XMFLOAT3(1, 1, 1), basicPShader, basicVShader, 0.0f);
	woodMat->AddTextureSRV(0, woodSRV);
	woodMat->AddSampler(0, samplerState);

	std::shared_ptr<Material> woodCrackMat = std::make_shared<Material>("Cracked Wood", XMFLOAT3(1, 1, 1), basicPShader, basicVShader, 0.0f);
	woodCrackMat->AddTextureSRV(0, woodSRV);
	woodCrackMat->AddSampler(0, samplerState);

	std::shared_ptr<Material> tileMat = std::make_shared<Material>("Tiles",XMFLOAT3(1, 1, 1), basicPShader, basicVShader, 0.0f);
	tileMat->AddTextureSRV(0, tilesSRV);
	tileMat->AddSampler(0, samplerState);

	std::shared_ptr<Material> fancyMat = std::make_shared<Material>("Fancy",XMFLOAT3(1, 1, 1), basicPShader, basicVShader, 1.0f);

	// create meshes
	std::shared_ptr<Mesh> cubeMesh = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/cube.obj").c_str(), "Cube");
	std::shared_ptr<Mesh> cylinderMesh = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/cylinder.obj").c_str(), "Cylinder");
	std::shared_ptr<Mesh> helixMesh = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/helix.obj").c_str(), "Helix");
	std::shared_ptr<Mesh> quadMesh = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/quad.obj").c_str(), "Quad");
	std::shared_ptr<Mesh> quad2SideMesh = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/quad_double_sided.obj").c_str(), "2 Sided Quad");
	std::shared_ptr<Mesh> sphereMesh = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/sphere.obj").c_str(), "Sphere");
	std::shared_ptr<Mesh> torusMesh = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/torus.obj").c_str(), "Torus");

	// add meshes to the vector 
	meshes.insert(meshes.end(), { cubeMesh, cylinderMesh, helixMesh, quadMesh, quad2SideMesh, sphereMesh, torusMesh });

	// create entities
	entities.push_back(std::make_shared<Entity>(cubeMesh, woodCrackMat));
	entities.push_back(std::make_shared<Entity>(cylinderMesh, woodMat));
	entities.push_back(std::make_shared<Entity>(helixMesh, woodMat));
	entities.push_back(std::make_shared<Entity>(quadMesh, tileMat));
	entities.push_back(std::make_shared<Entity>(quad2SideMesh, tileMat));
	entities.push_back(std::make_shared<Entity>(sphereMesh, tileMat));
	entities.push_back(std::make_shared<Entity>(torusMesh, tileMat));

	// Adjust transforms
	entities[0]->GetTransform()->MoveAbsolute(-9, 0, 0);
	entities[1]->GetTransform()->MoveAbsolute(-6, 0, 0);
	entities[2]->GetTransform()->MoveAbsolute(-3, 0, 0);
	entities[3]->GetTransform()->MoveAbsolute(0, 0, 0);
	entities[4]->GetTransform()->MoveAbsolute(3, 0, 0);
	entities[5]->GetTransform()->MoveAbsolute(6, 0, 0);
	entities[6]->GetTransform()->MoveAbsolute(9, 0, 0);

	// Lights 
	// Initialize Directional Light
	Light directionalLight1 = {};
	directionalLight1.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight1.direction = XMFLOAT3(1.0f, 0.0f, 0.0f);
	directionalLight1.color = XMFLOAT3(1.0f, 0, 0);
	directionalLight1.intensity = 1.0f;

	Light directionalLight2 = {};
	directionalLight2.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight2.direction = XMFLOAT3(-1.0f, 0.0f, 0.0f);
	directionalLight2.color = XMFLOAT3(0.0f, 1.0f, 0);
	directionalLight2.intensity = 1.0f;

	Light directionalLight3 = {};
	directionalLight3.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight3.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	directionalLight3.color = XMFLOAT3(0.0f, 0, 1.0f);
	directionalLight3.intensity = 1.0f;

	Light pointLight1 = {};
	pointLight1.type = LIGHT_TYPE_POINT;
	pointLight1.color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	pointLight1.intensity = 5.0f;
	pointLight1.range = 10.0f;
	pointLight1.position = XMFLOAT3(-4.5f, 0, 0);

	Light spotLight1 = {};
	spotLight1.type = LIGHT_TYPE_SPOT;
	spotLight1.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	spotLight1.color = XMFLOAT3(0.0f, 0, 1.0f);
	spotLight1.intensity = 2.0f;
	spotLight1.position = XMFLOAT3(0, 1.5f, 0);
	spotLight1.range = 10.0f;
	spotLight1.spotOuterAngle = XMConvertToRadians(30.0f);
	spotLight1.spotInnerAngle = XMConvertToRadians(20.0f);

	lights.push_back(directionalLight1);
	lights.push_back(directionalLight2);
	lights.push_back(directionalLight3);
	lights.push_back(pointLight1);
	lights.push_back(spotLight1);
}



// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	if (cameras[0]) {
		for (auto& camera : cameras) {
			camera->UpdateProjectionMatrix(Window::AspectRatio());
		}
	}
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

	/*for (auto& entity : entities) {
		entity->GetTransform()->Rotate(0, deltaTime, 0);
	}*/


	cameras[activeCameraIndex]->Update(deltaTime);
	globalPsData.time = totalTime;

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

	VertexShaderExternalData vsData = {};
	PixelShaderExternalData psData = {};

	psData.camPos = cameras[activeCameraIndex]->GetTransform()->GetPosition();
	psData.ambientColor = ambientColor;
	memcpy(&psData.lights, &lights[0], sizeof(Light) * (int)lights.size());
	

	// For each entity
	for (auto& entity : entities) {
		

		// set the world, view, and projection matrices
		vsData.world = entity->GetTransform()->GetWorldMatrix();
		vsData.worldInvTrans = entity->GetTransform()->GetWorldInverseTransposeMatrix();
		vsData.viewMatrix = cameras[activeCameraIndex]->GetView();
		vsData.projectionMatrix = cameras[activeCameraIndex]->GetProjection();

		Graphics::FillAndBindNextConstantBuffer(&vsData, sizeof(VertexShaderExternalData), D3D11_VERTEX_SHADER, 0);

		// set data
		psData.colorTint = entity->GetMaterial()->GetColorTint();
		psData.time = globalPsData.time;
		psData.uvOffset = entity->GetMaterial()->GetUVOffset();
		psData.uvScale = entity->GetMaterial()->GetUVScale();
		psData.roughness = entity->GetMaterial()->GetRoughness();

		// Draw entity
		Graphics::FillAndBindNextConstantBuffer(&psData, sizeof(PixelShaderExternalData), D3D11_PIXEL_SHADER, 0);

		

	
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

					if (ImGui::TreeNode("Material Node", "Material: %s", entities[i]->GetMaterial()->GetName())) {
						// Color tint editing
						XMFLOAT3 tint = entities[i]->GetMaterial()->GetColorTint();
						if (ImGui::ColorEdit3("Color Tint", &tint.x))
							entities[i]->GetMaterial()->SetColorTint(tint);

						// UV manipulations
						XMFLOAT2 uvScale = entities[i]->GetMaterial()->GetUVScale();
						XMFLOAT2 uvOffset = entities[i]->GetMaterial()->GetUVOffset();
						if (ImGui::DragFloat2("UV Scale", &uvScale.x, 0.25f)) entities[i]->GetMaterial()->SetUVScale(uvScale);
						if (ImGui::DragFloat2("UV Offset", &uvOffset.x, 0.05f)) entities[i]->GetMaterial()->SetUVOffset(uvOffset);

						// Textures
						for (auto& it : entities[i]->GetMaterial()->GetTextureSRVMap())
						{
							ImGui::Text("Texture %d", it.first);
							ImGui::Image(it.second.Get(), ImVec2(256, 256));
						}
						ImGui::TreePop();
					}

					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}

		// Lights
		if (ImGui::TreeNode("Lights")) {

			// Edit Ambient Term
			if (ImGui::TreeNode("Ambient Term")) {
				ImGui::ColorEdit3("Ambient Color", &ambientColor.x);
				ImGui::TreePop();
			}

			// Each Light Color and Intensity control
			for (int i = 0; i < lights.size(); i++) {
				ImGui::PushID(i);

				// Light Node
				if (ImGui::TreeNode("Light Node", "Light %d", i)) {

					// Controls
					ImGui::ColorEdit3("Color", &lights[i].color.x);
					ImGui::SliderFloat("Intensity", &lights[i].intensity, 0.0f, 10.0f);
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}


	}

	// Camera Collapsing Header
	if (ImGui::CollapsingHeader("Camera")) {
		ImGui::RadioButton("Camera 1", &activeCameraIndex, 0);
		ImGui::SameLine();
		ImGui::RadioButton("Camera 2", &activeCameraIndex, 1);
		ImGui::SameLine();
		ImGui::RadioButton("Camera 3", &activeCameraIndex, 2);

		if (ImGui::TreeNode("Camera Details")) {
			ImGui::Text("Position: x-%.2f y-%.2f z-%.2f", 
				cameras[activeCameraIndex]->GetTransform()->GetPosition().x,
				cameras[activeCameraIndex]->GetTransform()->GetPosition().y,
				cameras[activeCameraIndex]->GetTransform()->GetPosition().z
				);
			ImGui::Text("Fov (in degrees): %.0f", cameras[activeCameraIndex]->GetFieldOfView() * (180.0 / DirectX::XM_PI));
			ImGui::Text("Near Clip Distance: %.1f", cameras[activeCameraIndex]->GetNearClip());
			ImGui::Text("Far Clip Distance: %.1f", cameras[activeCameraIndex]->GetFarClip());
			switch (cameras[activeCameraIndex]->GetProjectionType())
			{
			case PERSPECTIVE:
				ImGui::Text("Projection Type: Perspective");
				break;
			case ORTHOGRAPHIC:
				ImGui::Text("Projection Type: Orthographic");
				ImGui::Text("Orthographic Width: %.2f", cameras[activeCameraIndex]->GetOrthoGraphicWidth());
				break;
			default:
				break;
			}
			ImGui::TreePop();
		}
	}


	ImGui::End();

}


Microsoft::WRL::ComPtr<ID3D11PixelShader> Game::LoadPixelShader(const std::wstring& fileName)
{
	// Read the file to a blob
	ID3DBlob* pixelShaderBlob;
	D3DReadFileToBlob(FixPath(fileName).c_str(), &pixelShaderBlob);

	// Create the shader and return it
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Graphics::Device->CreatePixelShader(
		pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
		pixelShaderBlob->GetBufferSize(),		// How big is that data?
		0,										// No classes in this shader
		pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

	return pixelShader;
}

Microsoft::WRL::ComPtr<ID3D11VertexShader> Game::LoadVertexShader(const std::wstring& fileName)
{
	// Read the file to a blob
	ID3DBlob* vertexShaderBlob;
	D3DReadFileToBlob(FixPath(fileName).c_str(), &vertexShaderBlob);

	// Create the shader and return it
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Graphics::Device->CreateVertexShader(
		vertexShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
		vertexShaderBlob->GetBufferSize(),		// How big is that data?
		0,										// No classes in this shader
		vertexShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

	return vertexShader;
}

