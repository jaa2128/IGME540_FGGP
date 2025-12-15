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

	void CreateShadowMapResources();
	void RenderShadowMap();

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

	// Shadow Map Data
	float shadowMapResolution = 1024;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
	DirectX::XMFLOAT4X4 lightViewMatrix;
	DirectX::XMFLOAT4X4 lightProjectionMatrix;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> shadowVS;

	// Resources that are shared among all post processes
	Microsoft::WRL::ComPtr<ID3D11SamplerState> ppSampler;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> ppVS;

	// Resources that are tied to a particular post process

	// Blur
	Microsoft::WRL::ComPtr<ID3D11PixelShader> blurPS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> blurRTV; // For rendering
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> blurSRV; // For sampling

	// Pixelation
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelPS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pixelRTV; // For rendering
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pixelSRV; // For sampling

	// Blur Data
	int blurDistance = 0;

	// Pixelation Data
	int pixelSize = 1;

	// post Process helper
	void ResizePostProcessResources();
};

