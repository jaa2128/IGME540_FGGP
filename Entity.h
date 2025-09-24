#pragma once
#include "Mesh.h"
#include "Transform.h"
#include <memory>
class Entity
{
public:
	Entity(std::shared_ptr<Mesh> _mesh);
	~Entity();

	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();

	void Draw();

private:
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Transform> transform;
};

