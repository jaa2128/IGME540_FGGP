#pragma once
#include "Input.h"
#include "Transform.h"
#include <DirectXMath.h>

enum ProjectionType {
	PERSPECTIVE,
	ORTHOGRAPHIC
};

class Camera
{
public:
private:
	// Camera core variables
	Transform transform;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	// Camera customization variables
	float fov;
	float nearClipDistance;
	float farClipDistance;
	float cameraSpeed;
	float mouseLookSpeed;
	ProjectionType projectionType;
};

