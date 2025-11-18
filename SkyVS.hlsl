#include "ShaderStructs.hlsli"
// Constant buffer bound to the 0 indexed buffer
// (b0); b = buffer; 0 = index
// name is arbitrary
// layout MUST match struct
cbuffer VertexShaderData : register(b0)
{
    matrix view;
    matrix projection;
}



// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel_Sky main(VertexShaderInput input)
{
	// Set up output struct
    VertexToPixel_Sky output;
    
    // Modify view matrix to have no translation
    matrix viewNoTranslation = view;
    viewNoTranslation._14 = 0;
    viewNoTranslation._24 = 0;
    viewNoTranslation._34 = 0;

    // Apply the projection and new View Matrix to the inputPosition
    matrix vp = mul(projection, viewNoTranslation);
    output.position = mul(vp, float4(input.localPosition, 1.0f));
    
    // ensure depth is exactly 1 after the shader by setting z = w
    output.position.z = output.position.w;
    
    // use vert's position as the sample direction
    output.sampleDir = input.localPosition;
	
    return output;
}