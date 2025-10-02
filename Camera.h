#pragma once
#include "Input.h"
#include "Transform.h"
#include <DirectXMath.h>
#include <memory>

enum ProjectionType {
	PERSPECTIVE,
	ORTHOGRAPHIC
};

class Camera
{
public:
	Camera(
		DirectX::XMFLOAT3 position,
		float _cameraSpeed, 
		float _mouseLookSpeed,
		float _fov,
		float aspectRatio, 
		float _nearClipDistance = 0.01f,
		float _farClipDistance,
		ProjectionType type = ProjectionType::PERSPECTIVE
	);

	~Camera();

	void UpdateProjectionMatrix(float aspectRatio);
	void UpdateViewMatrix();
	void Update(float dt);

	// getters
	DirectX::XMFLOAT4X4 GetView();
	DirectX::XMFLOAT4X4 GetProjection();
	std::shared_ptr<Transform> GetTransform();

	float GetAspectRatio();
	
	float GetFieldOfView();
	void SetFieldOfView(float _fov);

	float GetNearClip();
	void SetNearClip(float distance);

	float GetFarCLip();
	void SetFarClip(float distance);

	float GetOrthoGraphicWidth();
	void SetOrthoGraphicWidth(float width);
	
	ProjectionType GetProjectionType();
	void SetProjectionType(ProjectionType type);

	float GetCameraSpeed();
	void SetCameraSpeed(float speed);

	float GetMouseLookSpeed();
	void SetMouseLookSpeed(float speed);

private:
	// Camera core variables
	std::shared_ptr<Transform> transform;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	// Camera customization variables
	float fov;
	float aspectRatio;
	float nearClipDistance;
	float farClipDistance;
	float cameraSpeed;
	float mouseLookSpeed;
	float orthographicWidth;
	ProjectionType projectionType;
};

