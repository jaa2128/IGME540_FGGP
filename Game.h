#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include<memory>
#include "Mesh.h"
#include "BufferStructs.h"
#include "Entity.h"
#include "Camera.h"

class Game
{
public:
	// Basic OOP setup
	Game();
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders();
	void CreateGeometry();
	void RefreshUI(float deltaTime);
	void BuildUI();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Shaders and shader-related constructs
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	// Variables that must persist between frames
	float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f }; // background color

	// demo array for ImGui Combo (dropdown select)
	const char* flavors[6] = {"Vanilla", "Chocolate", "Strawberry", "Neopolitan", "Mint", "Rocky Road"};

	// array of Mesh Objects
	std::vector<std::shared_ptr<Mesh>> meshes;

	// array of Entity Objects
	std::vector <std::shared_ptr<Entity>> entities;

	// Game's Constant Buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

	// Buffer Struct to be mapped and modified by the UI
	VertexShaderExternalData globalVsData = {};

	// Camera
	std::shared_ptr<Camera> camera;
};

