Texture2D mytexture : register(t0);
TextureCube skyBox : register(t1);
SamplerState samplerState : register(s0);

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
    float4 surface = mytexture.Sample(samplerState, input.uvw.xy);
    float4 cameraPos = /*camera_pos*/ { 5, 0, -1, 0 };
    
    float3 reflection = float3(0.0f, 0.0f, 0.0f);
    float4 rColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    reflection = reflect((input.posW - cameraPos.xyz), input.nrm);
    rColor = skyBox.Sample(samplerState, reflection);
    
    // Directional light
    float3 dLightPosition = float3(-1, -1, 5);
    float dLightRatio = saturate(dot(-normalize(dLightPosition), normalize(input.nrm)));
    float dIntensity = 1.0f;
    float4 dColor = float4(0.5f, 0.5f, 0.0f, 1) * dIntensity;
    float4 dLightResult = dLightRatio * dColor;
    
    // Point Light
    float3 pLightPosition = normalize(p_pos.xyz - input.posW);
    float attenuation = 1.0 - saturate(length(pLightPosition.xyz - input.nrm) / 2.0f);
    float pIntensity = 1.0f;
    float4 pColor = p_rgba * pIntensity;
    float4 pLightResult = (attenuation * attenuation) * pColor;
    
    // Spot Light
    float3 sLightPosition = float3(0, 10, 0);
    float3 coneDirection = float3(0, -1, 0);
    float2 coneAngle = float2(0.93f, 0.99f);
    float sIntensity = 1.0f;
    float4 sColor = float4(1, 0, 1, 1) * sIntensity;
    float3 lightDir = normalize(sLightPosition - input.posW);
    float surfaceRatio = saturate(dot(-lightDir, coneDirection));
    float4 spotFac = (surfaceRatio > coneAngle.x) ? 1 : 0;
    float4 lightRatio = saturate(dot(input.nrm, lightDir));
    attenuation = 1 - saturate((coneAngle.y - surfaceRatio) / (coneAngle.y - coneAngle.x));
    float4 sLightResult = (attenuation * attenuation) * spotFac * lightRatio * sColor;
    
    // Reflection Maping
    float3 h = normalize(normalize(cameraPos.xyz - input.posW) - (pLightPosition - input.posW) - (dLightPosition - input.posW) - (sLightPosition - input.posW));
    float specIntensity = 0.25f;
    float specLighting = (pow(saturate(dot(h, input.nrm)), 2.0f) + saturate(pColor * sColor * dColor)) * specIntensity * rColor;
    
    surface.a = 1.0f;
    float4 ambiant = { 0.75, 0.75, 0.75, 1 };
    float4 color = (ambiant * skyBox.Sample(samplerState, reflection)) * saturate(surface + dLightResult + pLightResult + sLightResult + specLighting);
    
    return color;
}