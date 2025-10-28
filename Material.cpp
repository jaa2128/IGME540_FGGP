#include "Material.h"
#include "Graphics.h"

Material::Material(const char* _name, DirectX::XMFLOAT3 _colorTint, Microsoft::WRL::ComPtr<ID3D11PixelShader> _pixelShader, 
    Microsoft::WRL::ComPtr<ID3D11VertexShader> _vertexShader, DirectX::XMFLOAT2 _uvScale, 
    DirectX::XMFLOAT2 _uvOffset) : colorTint(_colorTint), pixelShader(_pixelShader),
    vertexShader(_vertexShader), uvScale(_uvScale), uvOffset(_uvOffset), name(_name)
{
}

Material::~Material()
{
}

const char* Material::GetName()
{
    return name;
}

std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& Material::GetTextureSRVMap()
{
    return textureSRVs;
}

std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11SamplerState>>& Material::GetSamplerMap()
{
    return samplers;
}

DirectX::XMFLOAT3 Material::GetColorTint()
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

DirectX::XMFLOAT2 Material::GetUVOffset()
{
    return uvOffset;
}

DirectX::XMFLOAT2 Material::GetUVScale()
{
    return uvScale;
}

void Material::SetColorTint(DirectX::XMFLOAT3 _colorTint)
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

void Material::SetUVScale(DirectX::XMFLOAT2 scale)
{
    uvScale = scale;
}

void Material::SetUVOffset(DirectX::XMFLOAT2 offset)
{
    uvOffset = offset;
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
