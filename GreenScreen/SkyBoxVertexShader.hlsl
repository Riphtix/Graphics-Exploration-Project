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
};

struct SkyBoxVertex_OUT
{
    float4 posH : SV_POSITION;
    float3 uvw : TEXCOORD;
    float3 nrm : NORMAL;
    float3 posW : WORLD;
};

SkyBoxVertex_OUT main(Vertex_IN input)
{
    SkyBoxVertex_OUT output;
    
    output.posH = float4(input.posL, 1);
    output.posH = mul(world, output.posH);
    output.posW = output.posH.xyz;
    output.posH = mul(view, output.posH);
    output.posH = mul(projection, output.posH);
    
    output.nrm = mul(world, float4(input.nrm, 0)).xyz;
    output.uvw = input.posL;
    
    return output;
}