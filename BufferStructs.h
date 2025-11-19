#pragma once
#include <DirectXMath.h>
#include "Lights.h"

struct VertexShaderExternalData {
	DirectX::XMFLOAT4X4 world; // 3 floats = 12 bytes
	DirectX::XMFLOAT4X4 worldInvTrans;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;
	

	// Total bytes: 28 bytes
};

struct PixelShaderExternalData {
	DirectX::XMFLOAT3 colorTint;
	float time;						// 16 bytes total aligned
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;		// 32 bytes total alighned
	float roughness;				
	DirectX::XMFLOAT3 camPos;		// 48 bytes total aligned
	DirectX::XMFLOAT3 ambientColor; 
	float pad;
	Light lights[5];
};

struct SkyBoxExternalData {
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;
};
