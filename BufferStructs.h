#pragma once
#include <DirectXMath.h>

struct VertexShaderExternalData {
	DirectX::XMFLOAT4X4 world; // 3 floats = 12 bytes
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;
	

	// Total bytes: 28 bytes
};

struct PixelShaderExternalData {
	DirectX::XMFLOAT3 colorTint;
	float time;
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;
};
