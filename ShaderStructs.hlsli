#ifndef SHADER_STRUCTS__ // Each .hlsli file needs a unique identifier!
#define SHADER_STRUCTS__
// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD; // Object UV
    float3 normal : NORMAL; // Object Normals
    float3 tangent : TANGENT;
    float3 worldPos : POSITION;
};

#endif