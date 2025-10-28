Texture2D SurfaceTexture : register(t0); // "t" registers for textures
Texture2D OtherTexture : register(t1); // register 1
SamplerState BasicSampler : register(s0); // "s" registers for samplers
// Constant buffer bound to the 0 indexed buffer
// (b0); b = buffer; 0 = index
// name is arbitrary
// layout MUST match struct
cbuffer PixelShaderData : register(b0)
{
    float3 colorTint;
    float time;
    float2 uvScale;
    float2 uvOffset;
}

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
};

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	// Adjust UV coords
    input.uv = input.uv * uvScale + uvOffset;
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
    float3 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv).rgb;
    surfaceColor *= colorTint;
    surfaceColor *= OtherTexture.Sample(BasicSampler, input.uv).rgb;
    return float4(surfaceColor, 1);
}