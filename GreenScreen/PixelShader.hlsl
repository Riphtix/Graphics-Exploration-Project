Texture2D mytexture : register(t0);
sampler quality : register(s0);

struct Vertex_OUT
{
    float4 posH : SV_POSITION;
    float3 uvw : TEXCOORD;
    float3 nrm : NORMAL;
};

// an ultra simple hlsl pixel shader
float4 main(Vertex_OUT input) : SV_TARGET
{
    float4 surface = mytexture.Sample(quality, input.uvw.xy);
    float lightRatio = dot(normalize(input.nrm), -normalize(float3(-1, -1, 1)));
    float l_r = 0, l_g = 0, l_b = 255;
    return surface * lightRatio + float4(l_r / 255, l_g / 255, l_b / 255, 1);
	//return float4(input.uvw, 0); float4(200/255.0f,150/255.0f,8/255.0f,0); 
}