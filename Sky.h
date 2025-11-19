#pragma once

#include "Mesh.h"
#include "Camera.h"

#include <memory>
#include <wrl/client.h> // Used for ComPtr


class Sky
{
public:
	Sky(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back,
		std::shared_ptr<Mesh> _mesh,
		Microsoft::WRL::ComPtr<ID3D11VertexShader> _skyVS,
		Microsoft::WRL::ComPtr<ID3D11PixelShader> _skyPS,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> _sampler
	);

	void Draw(std::shared_ptr<Camera> camera);

private:

	// Helper for creating a cubemap from 6 individual textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);

	Microsoft::WRL::ComPtr<ID3D11VertexShader> skyVS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> skyPS;

	std::shared_ptr<Mesh> skyMesh;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> skyRasterState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> skyDepthState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skySRV;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
};

