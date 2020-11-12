Texture2D mytexture : register(t0);
sampler quality : register(s0);

struct Vertex_OUT
{
    float4 posH : SV_POSITION;
    float3 uvw : TEXCOORD;
    float3 nrm : NORMAL;
    float3 posW : WORLD;
};

// an ultra simple hlsl pixel shader
float4 main(Vertex_OUT input) : SV_TARGET
{
    float4 surface = mytexture.Sample(quality, input.uvw.xy);
    
    // Ambiant light
    float3 aLightPosition = float3(-1, -1, 5);
    float aLightRatio = saturate(dot(-normalize(aLightPosition), normalize(input.nrm)));
    float4 aColor = float4(0.1f, 0.1f, 0.1f, 1);
    float4 aLightResult = aLightRatio * aColor;
    
    // Directional light
    float3 dLightPosition = float3(-1, -1, 5);
    float dLightRatio = saturate(dot(-normalize(dLightPosition), normalize(input.nrm)));
    float dIntensity = 0.3f;
    float4 dColor = float4(0.0f, 0.0f, 1.0f, 1) * dIntensity;
    float4 dLightResult = dLightRatio * dColor;
    
    // Point Light
    float3 pLightPosition = float3(5, 0, 0) - input.posW;
    float attenuation = 1.0 - saturate(length(normalize(pLightPosition) - normalize(input.nrm)) / 2.0f);
    float pIntensity = 0.3f;
    float4 pColor = float4(0.0f, 1.0f, 0.0f, 1) * pIntensity;
    float4 pLightResult = (attenuation * attenuation) * pColor;
    
    // Spot Light
    float3 sLightPosition = float3(0, 10, 0);
    float3 coneDirection = float3(0, -1, 0);
    float2 coneAngle = float2(0.93f, 0.99f);
    float sIntensity = 0.3f;
    float4 sColor = float4(1, 0, 0, 1) * sIntensity;
    float3 lightDir = normalize(sLightPosition - input.posW);
    float surfaceRatio = saturate(dot(-lightDir, coneDirection));
    float4 spotFac = (surfaceRatio > coneAngle.x) ? 1 : 0;
    float4 lightRatio = saturate(dot(input.nrm, lightDir));
    attenuation = 1 - saturate((coneAngle.y - surfaceRatio) / (coneAngle.y - coneAngle.x));
    float4 sLightResult = (attenuation * attenuation) * spotFac * lightRatio * sColor;
    
    // Reflection Maping
    float4 cameraPos = { 2, 1.5, -1, 0 };
    float3 h = normalize(normalize(cameraPos.xyz - input.posW) - (pLightPosition - input.posW) - (dLightPosition - input.posW) - (sLightPosition - input.posW));
    float specIntensity = 0.25f;
    float specLighting = (pow(saturate(dot(h, input.nrm)), 2.0f) + saturate(pColor * sColor * dColor)) * specIntensity;
    
    //float lightRatio = dot(normalize(input.nrm), -normalize(float3(-1, -1, 1)));
    //float l_r = 0, l_g = 0, l_b = 255;
    
    surface.a = 1.0f;
    float4 baseColor = { 1, 1, 1, 1 };
    float4 ambiant = { 0.1, 0.1, 0.1, 1 };
    float4 color = saturate(surface + dLightResult + pLightResult + sLightResult + specLighting);
    return color; //surface * lightRatio + float4(l_r / 255, l_g / 255, l_b / 255, 1);
	//return float4(input.uvw, 0); float4(200/255.0f,150/255.0f,8/255.0f,0); 
}