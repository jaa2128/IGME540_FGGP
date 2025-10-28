#pragma once
#include <DirectXMath.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <unordered_map>

class Material
{
public:
	Material(DirectX::XMFLOAT3 _colorTint, Microsoft::WRL::ComPtr<ID3D11PixelShader> _pixelShader, 
		Microsoft::WRL::ComPtr<ID3D11VertexShader>  _vertexShader, 
		DirectX::XMFLOAT2 _uvScale = DirectX::XMFLOAT2(1, 1), DirectX::XMFLOAT2 _uvOffset = DirectX::XMFLOAT2(0, 0));
	~Material();

	// Getters
	DirectX::XMFLOAT3 GetColorTint();
	Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPixelShader();
	Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVertexShader();
	DirectX::XMFLOAT2 GetUVOffset();
	DirectX::XMFLOAT2 GetUVScale();

	// Setters 
	void SetColorTint(DirectX::XMFLOAT3 _colorTint);
	void SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> _pixelShader);
	void SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> _vertexShader);
	void SetUVScale(DirectX::XMFLOAT2 scale);
	void SetUVOffset(DirectX::XMFLOAT2 offset);

	// Texture and Sampler methods
	void AddTextureSRV(unsigned int index, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(unsigned int index, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);
	void BindTexturesAndSamplers();

private:
	DirectX::XMFLOAT3 colorTint;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;

	// Texture Stuff
	DirectX::XMFLOAT2 uvOffset;
	DirectX::XMFLOAT2 uvScale;
	std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;

};

