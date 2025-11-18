#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Vertex.h"
#include "Graphics.h"
#include <string>

class Mesh
{
public:
	/// <summary>
	/// Mesh Constructor
	/// </summary>
	/// <param name="vertices">Array of Vertices</param>
	/// <param name="indices">Array of Indices used by Mesh</param>
	/// <param name="numVertices">Number of Vertices in Array</param>
	Mesh(Vertex vertices[], unsigned int indices[], unsigned int _numVertices, unsigned int _numIndices, const char* _name);
	Mesh(const char* fileName, const char* _name);
	~Mesh();

	// Getters for Vertex and Index Buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();

	// Getters for Index and Vertex Counts
	int GetIndexCount();
	int GetVertexCount();
	
	const char* GetName();
	
	int CalculateTris();

	// Draw
	void Draw();


private:
	void CreateBuffers(Vertex vertices[], unsigned int indices[], unsigned int _numVertices, unsigned int _numIndices);
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer; // Vertex Buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer; // Index Buffer

	int numIndices; // Number of Indices
	int numVertices; // Number of Vertices

	const char* name;
};

