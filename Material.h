#pragma once
#include <DirectXMath.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <unordered_map>

class Material
{
public:
	Material(DirectX::XMFLOAT4 _colorTint, Microsoft::WRL::ComPtr<ID3D11PixelShader> _pixelShader, 
		Microsoft::WRL::ComPtr<ID3D11VertexShader>  _vertexShader);
	~Material();

	// Getters
	DirectX::XMFLOAT4 GetColorTint();
	Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPixelShader();
	Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVertexShader();

	// Setters 
	void SetColorTint(DirectX::XMFLOAT4 _colorTint);
	void SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> _pixelShader);
	void SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> _vertexShader);

	// Texture and Sampler methods
	void AddTextureSRV(unsigned int index, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(unsigned int index, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);
	void BindTexturesAndSamplers();

private:
	DirectX::XMFLOAT4 colorTint;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;

	std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;

};

