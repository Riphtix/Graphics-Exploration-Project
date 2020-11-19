cbuffer Instanced_Shader_Variables : register(b0)
{
    float4x4 world[3];
    float4x4 view;
    float4x4 projection;
};

struct Vertex_IN
{
    float3 posL : POSITION;
    float3 uvw : TEXCOORD;
    float3 nrm : NORMAL;
};

struct Vertex_OUT
{
    float4 posH : SV_POSITION;
    float3 uvw : TEXCOORD;
    float3 nrm : NORMAL;
    float3 posW : WORLD;
};

// an ultra simple hlsl vertex shader
Vertex_OUT main(Vertex_IN input, uint id : SV_InstanceID)
{
    Vertex_OUT output;
	
    output.posH = float4(input.posL, 1);
    output.posH = mul(world[id], output.posH);
    output.posW = output.posH.xyz;
    output.posH = mul(view, output.posH);
    output.posH = mul(projection, output.posH);

    output.uvw = input.uvw;
    output.nrm = mul(world[id], float4(input.nrm, 0)).xyz;

    return output;
}