#include "Material.h"
#include "Graphics.h"

Material::Material(DirectX::XMFLOAT4 _colorTint, Microsoft::WRL::ComPtr<ID3D11PixelShader> _pixelShader, 
    Microsoft::WRL::ComPtr<ID3D11VertexShader> _vertexShader) : colorTint(_colorTint), pixelShader(_pixelShader), 
    vertexShader(_vertexShader)
{
}

Material::~Material()
{
}

DirectX::XMFLOAT4 Material::GetColorTint()
{
    return colorTint;
}

Microsoft::WRL::ComPtr<ID3D11PixelShader> Material::GetPixelShader()
{
    return pixelShader;
}

Microsoft::WRL::ComPtr<ID3D11VertexShader> Material::GetVertexShader()
{
    return vertexShader;
}

void Material::SetColorTint(DirectX::XMFLOAT4 _colorTint)
{
    colorTint = _colorTint;
}

void Material::SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> _pixelShader)
{
    pixelShader = _pixelShader;
}

void Material::SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> _vertexShader)
{
    vertexShader = _vertexShader;
}

void Material::AddTextureSRV(unsigned int index, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
    textureSRVs.insert({ index, srv });
}


void Material::AddSampler(unsigned int index, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
    samplers.insert({ index, sampler });
}

void Material::BindTexturesAndSamplers()
{
    for (auto& t : textureSRVs) {
        Graphics::Context->PSSetShaderResources(t.first, 1, t.second.GetAddressOf()); 
    }

    for (auto& s : samplers) { 
        Graphics::Context->PSSetSamplers(s.first, 1, s.second.GetAddressOf()); 
    }
}
