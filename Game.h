#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include<memory>
#include "Mesh.h"
#include "BufferStructs.h"
#include "Entity.h"
#include "Camera.h"
#include <string>
#include "Lights.h"
#include "Sky.h"

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
	void LoadAssetsAndCreateEntities();
	void RefreshUI(float deltaTime);
	void BuildUI();

	Microsoft::WRL::ComPtr<ID3D11PixelShader> LoadPixelShader(const std::wstring& fileName);
	Microsoft::WRL::ComPtr<ID3D11VertexShader> LoadVertexShader(const std::wstring& fileName);

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Shaders and shader-related constructs
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	// Variables that must persist between frames
	float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f }; // background color
	DirectX::XMFLOAT3 ambientColor = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

	// demo array for ImGui Combo (dropdown select)
	const char* flavors[6] = {"Vanilla", "Chocolate", "Strawberry", "Neopolitan", "Mint", "Rocky Road"};

	// array of Mesh Objects
	std::vector<std::shared_ptr<Mesh>> meshes;

	// array of Entity Objects
	std::vector <std::shared_ptr<Entity>> entities;

	// Buffer Struct to be mapped and modified by the UI
	VertexShaderExternalData globalVsData = {};
	PixelShaderExternalData globalPsData = {};

	// Camera
	std::vector<std::shared_ptr<Camera>> cameras;
	
	// int to store active camera index in vector
	int activeCameraIndex;

	// array of lights 
	std::vector<Light> lights;

	// Sky box
	std::shared_ptr<Sky> sky;
};

