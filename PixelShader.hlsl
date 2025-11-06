#include "ShaderStructs.hlsli"
#include "Lighting.hlsli"

Texture2D SurfaceTexture : register(t0); // "t" registers for textures
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
    float roughness;
    float3 camPos;
    float3 ambientColor;
    float pad;
    Light lights[5];
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
	// Adjust UV coords
    input.uv = input.uv * uvScale + uvOffset;
    
    // normalize input
    input.normal = normalize(input.normal);

    // Texture color
    float3 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv).rgb;
    surfaceColor *= colorTint;
    float3 ambient = ambientColor * surfaceColor;
    
    float3 totalLight = ambient;
    
    // Calculating Diffuse Lighting
    for (int i = 0; i < 5; i++)
    {
        Light light = lights[i];
        totalLight += DirectionalLight(light, input.normal, input.worldPos, camPos, roughness, surfaceColor);
    }
        
    return float4(totalLight, 1);
}