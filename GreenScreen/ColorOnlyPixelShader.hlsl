struct Vertex_OUT
{
    float4 posH : SV_POSITION;
    float3 uvw : TEXCOORD;
    float3 nrm : NORMAL;
    float3 posW : WORLD;
};

cbuffer P_Light : register(b0)
{
    float4 p_pos;
    float4 p_rgba;
    float4 p_radius;
    float4 camera_pos;
}

float4 main(Vertex_OUT input) : SV_TARGET
{
    return p_rgba;
}