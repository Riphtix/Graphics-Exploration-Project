TextureCube skyBox : register(t1);
SamplerState samplerState : register(s0);

struct SkyBoxVertex_OUT
{
    float4 posH : SV_POSITION;
    float3 uvw : TEXCOORD;
    float3 nrm : NORMAL;
    float3 posW : WORLD;
};

float4 main(SkyBoxVertex_OUT input) : SV_TARGET
{
    return skyBox.Sample(samplerState, input.uvw);
}