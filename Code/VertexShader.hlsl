// Rule of three
// Three things must match (Data)
// 1. C++ Vertex Struct
// 2. Input Layout
// 3. HLSL Vertex Struct

struct InputVertex
{
    float4 xyzw : POSITION;
    float4 rgba : COLOR;
};

struct OutputVertex
{
    float4 xyzw : SV_POSITION; // SV = System Value
    float4 rgba : OUT_COLOR;
};

cbuffer SHADER_VARIABLES : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

OutputVertex main(InputVertex input)
{
    OutputVertex output = (OutputVertex) 0;
    output.xyzw = input.xyzw;
    output.rgba = input.rgba;
    
    // Do math here (shader intrinsics)
    output.xyzw = mul(worldMatrix, output.xyzw);
    output.xyzw = mul(viewMatrix, output.xyzw);
    output.xyzw = mul(projectionMatrix, output.xyzw);
    // Don't do perspective divide
    
    return output;
}