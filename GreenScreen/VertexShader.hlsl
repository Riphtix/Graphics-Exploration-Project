cbuffer SHADER_VARIABLES : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
};

struct Vertex_IN
{
    float3 posL : POSITION;
    float3 uvw : TEXCOORD;
    float3 nrm : NORMAL;
    //uint instanceID : SV_InstanceID;
};

struct Vertex_OUT
{
    float4 posH : SV_POSITION;
    float3 uvw : TEXCOORD;
    float3 nrm : NORMAL;
};

// an ultra simple hlsl vertex shader
Vertex_OUT main(Vertex_IN input)
{    
    Vertex_OUT output;
    output.posH = float4(input.posL/* + input.instanceID*/, 1);
    output.posH = mul(world, output.posH);
    output.posH = mul(view, output.posH);
    output.posH = mul(projection, output.posH);

    output.uvw = input.uvw;
    output.nrm = mul(world, float4(input.nrm, 0)).xyz;
    
    //output.posH = sin(output.posH);

    return output;
}