
// Note: The Custom Shader I'm making is a remake of one I made for IGME 530
// This was originally made in p5 and thus written originally for glsl
// Just wanted to make note of this to avoid self plagiarism
// Here's the link to the shader I made https://editor.p5js.org/Blaze6000dgs/sketches/C4imb8O3J

// Constant buffer bound to the 0 indexed buffer
// (b0); b = buffer; 0 = index
// name is arbitrary
// layout MUST match struct
cbuffer PixelShaderData : register(b0)
{
    float4 colorTint;
    float time;
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

// Utility Functions
float random(float y)
{
    return frac(sin(y * 78.233) * 43758.5453);
}

float random(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453123);
}

// 2D Noise based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise(float2 uv)
{
    float2 i = floor(uv);
    float2 f = frac(uv);
    
    float a = random(i);
    float b = random(i + float2(1.0, 0.0));
    float c = random(i + float2(0.0, 1.0));
    float d = random(i + float2(1.0, 0.0));
    
    float2 u = f * f * (3.0 - 2.0 * f);
    
    return lerp(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;

}

// Tile function (creates staggered moving rows)
float2 tile(float2 uv, float zoom, float time)
{
    uv *= zoom;
    
    float rowIndex = floor(uv.y);
    float rowShift = fmod(rowIndex, 2.0);
    
    float speed = lerp(0.5, 10.0, random(rowIndex));
    float offset = (rowShift > 0.5) ? time * speed : -time * speed;
    
    uv.x += rowShift * 0.5;
    uv.x += offset;
    
    return frac(uv);
}

float boxShape(float2 uv, float2 size)
{
    float2 halfSize = float2(0.5, 0.5) - size * 0.5;
    float2 edge1 = step(halfSize, uv);
    float2 edge2 = step(halfSize, 1.0 - uv);
    return edge1.x * edge1.y * edge2.x * edge2.y;

}

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
    float2 uv = input.uv;
    float3 color = float3(0, 0, 0);
    
    // Base palette colors
    float3 colorA = float3(0.44, 0.0, 0.0);
    float3 colorB = float3(1.0, 0.8, 0.0);
    float3 colorC = float3(0.7, 0.0, 0.5);
    
    // Tile Space
    uv = tile(uv, 25.0, time);
    
    // Wavy distortion
    float waveStrength = 0.5;
    float2 wave = float2(noise(uv * 5.0 + time), noise(uv * 5.0 - time));
    uv += (wave - 0.5) * waveStrength;
    
    // Box Mask
    float boxMask = boxShape(uv, float2(0.5, 0.5));
    
    // Color Mixing
    color = lerp(colorA, colorB, abs(sin(time)));
    color = lerp(color, colorC, abs(sin(time)));
    color *= boxMask;
    
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
    return float4(color, 1);
}