// minimalistic code to draw a single triangle, this is not part of the API.
// required for compiling shaders on the fly, consider pre-compiling instead
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include "cube.h"
#include "9cube.h"
#include "sphere.h"
#include "DDSTextureLoader.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include <ctime>

// Simple Vertex Shader
const char* vertexShaderSource = R"(
cbuffer SHADER_VARIABLES : register(b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 projection;
	//float4x4 lightView;
	//float4x4 lightProjection;
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
	//float4 lvp : TEXCOORD1;
	//float3 lp : TEXCOORD2;
};

// an ultra simple hlsl vertex shader
Vertex_OUT main(Vertex_IN input)
{
	Vertex_OUT output;

	//uvw.w = 1.0f;
	
	output.posH = float4(input.posL, 1);
	output.posH = mul(world, output.posH);
	output.posW = output.posH;
	output.posH = mul(view, output.posH);
	output.posH = mul(projection, output.posH);

	//output.lvp = mul(world, input

	output.uvw = input.uvw;
	output.nrm = mul(world, float4(input.nrm, 0)).xyz;

	return output;
}
)";

// Simple Pixel Shader
//const char* pixelShaderSource = R"(
//Texture2D mytexture : register(t0);
//sampler quality : register(s0);
//
//struct Vertex_OUT
//{
//	float4 posH : SV_POSITION;
//	float3 uvw : TEXCOORD;
//	float3 nrm : NORMAL;
//};
//
//// an ultra simple hlsl pixel shader
//float4 main(Vertex_OUT input) : SV_TARGET 
//{	
//	return mytexture.Sample(quality, input.uvw.xy);
//	//return float4(input.uvw, 0); //float4(200/255.0f,150/255.0f,8/255.0f,0); 
//}
//)";

// Creation, Rendering & Cleanup
class Triangle
{
	// math things
	struct SHADER_VARIABLES {
		GW::MATH::GMATRIXF world = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF view = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF projection = GW::MATH::GIdentityMatrixF;
		//GW::MATH::GMATRIXF lightView = GW::MATH::GIdentityMatrixF;
		//GW::MATH::GMATRIXF lightProjection = GW::MATH::GIdentityMatrixF;
	}Send2Shader;

	// handle for math lib
	GW::MATH::GMatrix m;

	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;

	// what we need at a minimum to draw a triangle
	Microsoft::WRL::ComPtr<ID3D11Buffer>		cubeVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		cubeIndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		cubeVertexBuffer1;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		constantBuffer;

	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;

	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>  samplerState;

	// for texuring
	Microsoft::WRL::ComPtr<ID3D11Texture2D>		texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
public:
	Triangle(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		win = _win;
		d3d = _d3d;
		ID3D11Device* creator;
		d3d.GetDevice((void**)&creator);

		// Create Vertex Buffer
		/*float verts[] = {
			   0,   0.5f,
			 0.5f, -0.5f,
			-0.5f, -0.5f
		};*/
		D3D11_SUBRESOURCE_DATA cubeBData = { _9cube_data, 0, 0 };
		CD3D11_BUFFER_DESC cubeBDesc(sizeof(_9cube_data), D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&cubeBDesc, &cubeBData, cubeVertexBuffer.GetAddressOf());

		D3D11_SUBRESOURCE_DATA cubeIData = { _9cube_indicies, 0, 0 };
		CD3D11_BUFFER_DESC cubeIDesc(sizeof(_9cube_indicies), D3D11_BIND_INDEX_BUFFER);
		creator->CreateBuffer(&cubeIDesc, &cubeIData, cubeIndexBuffer.GetAddressOf());

		D3D11_SAMPLER_DESC samplerDesc;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = 10;
		creator->CreateSamplerState(&samplerDesc, &samplerState);

		// Create Vertex Shader
		UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		compilerFlags |= D3DCOMPILE_DEBUG;
#endif
		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, errors;
		if (SUCCEEDED(D3DCompile(vertexShaderSource, strlen(vertexShaderSource),
			nullptr, nullptr, nullptr, "main", "vs_4_0", compilerFlags, 0,
			vsBlob.GetAddressOf(), errors.GetAddressOf())))
		{
			creator->CreateVertexShader(vsBlob->GetBufferPointer(),
				vsBlob->GetBufferSize(), nullptr, vertexShader.GetAddressOf());
		}
		else
			std::cout << (char*)errors->GetBufferPointer() << std::endl;
		//creator->CreateVertexShader(VertexShader, sizeof(VertexShader), nullptr, vertexShader.GetAddressOf());

		// Create Pixel Shader
		/*Microsoft::WRL::ComPtr<ID3DBlob> psBlob; errors.Reset();
		if (SUCCEEDED(D3DCompile(pixelShaderSource, strlen(pixelShaderSource),
			nullptr, nullptr, nullptr, "main", "ps_4_0", compilerFlags, 0,
			psBlob.GetAddressOf(), errors.GetAddressOf())))
		{
			creator->CreatePixelShader(psBlob->GetBufferPointer(),
				psBlob->GetBufferSize(), nullptr, pixelShader.GetAddressOf());
		}
		else
			std::cout << (char*)errors->GetBufferPointer() << std::endl;*/
		creator->CreatePixelShader(PixelShader, sizeof(PixelShader), nullptr, pixelShader.GetAddressOf());

		// Create Input Layout
		D3D11_INPUT_ELEMENT_DESC format[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			//{ "SV_InstanceID", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA}
		};
		creator->CreateInputLayout(format, ARRAYSIZE(format),
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
			vertexFormat.GetAddressOf());

		// free temporary handle
		creator->Release();

		// math setup here
		m.Create();

		GW::MATH::GVECTORF eye = { 5, 0, -1, 0 };
		GW::MATH::GVECTORF at = { 0, 0, 0, 0 };
		GW::MATH::GVECTORF up = { 0, 1, 0, 0 };
		m.LookAtLHF(eye, at, up, Send2Shader.view);

		D3D11_SUBRESOURCE_DATA cData = { &Send2Shader, 0, 0 };
		CD3D11_BUFFER_DESC cDesc(sizeof(Send2Shader), D3D11_BIND_CONSTANT_BUFFER);
		creator->CreateBuffer(&cDesc, &cData, constantBuffer.GetAddressOf());

		// load texture into VRAM
		CreateDDSTextureFromFile(creator, L"../metal.dds", (ID3D11Resource**)texture.GetAddressOf(), srv.GetAddressOf());
	}
	void Render()
	{
		// modify world
		m.RotationYF(Send2Shader.world, 3.14 / 1000.0f, Send2Shader.world);

		//float keyPress = 0;
		//cameraController.GetState(G_KEY_W, keyPress);
		//if (keyPress > 0) {
		//	GW::MATH::GVECTORF adj = { 0.0f, 0.0f, -0.1f, 1.0f }; // forward = -0.1f z backward = 0.1f z left = 0.1f x right = -0.1f x
		//	m.TranslatelocalF(Send2Shader.view, adj, Send2Shader.view);
		//}

		//cameraController.GetState(G_KEY_S, keyPress);
		//if (keyPress > 0) {
		//	GW::MATH::GVECTORF adj = { 0.0f, 0.0f, 0.1f, 1.0f };
		//	m.TranslatelocalF(Send2Shader.view, adj, Send2Shader.view);
		//}

		//cameraController.GetState(G_KEY_A, keyPress);
		//if (keyPress > 0) {
		//	GW::MATH::GVECTORF adj = { 0.1f, 0.0f, 0.0f, 1.0f };
		//	m.TranslatelocalF(Send2Shader.view, adj, Send2Shader.view);
		//}

		//cameraController.GetState(G_KEY_D, keyPress);
		//if (keyPress > 0) {
		//	GW::MATH::GVECTORF adj = { -0.1f, 0.0f, 0.0f, 1.0f };
		//	m.TranslatelocalF(Send2Shader.view, adj, Send2Shader.view);
		//}

		//cameraController.GetState(G_KEY_Q, keyPress);
		//if (keyPress > 0) {
		//	GW::MATH::GVECTORF adj = { 0.0f, -0.1f, 0.0f, 1.0f };
		//	m.TranslatelocalF(Send2Shader.view, adj, Send2Shader.view);
		//}

		//cameraController.GetState(G_KEY_E, keyPress);
		//if (keyPress > 0) {
		//	GW::MATH::GVECTORF adj = { 0.0f, 0.1f, 0.0f, 1.0f };
		//	m.TranslatelocalF(Send2Shader.view, adj, Send2Shader.view);
		//}

		// aspect ratio
		float ar;
		d3d.GetAspectRatio(ar);
		m.ProjectionDirectXLHF(G_PI / 2, ar, 0.01f, 1000.0f, Send2Shader.projection);

		// grab the context & render target
		ID3D11DeviceContext* con;
		ID3D11RenderTargetView* view;
		ID3D11DepthStencilView* depth;
		d3d.GetImmediateContext((void**)&con);
		d3d.GetRenderTargetView((void**)&view);
		d3d.GetDepthStencilView((void**)&depth);

		// setup the pipeline
		ID3D11RenderTargetView* const views[] = { view };
		con->OMSetRenderTargets(ARRAYSIZE(views), views, depth);
		const UINT strides[] = { sizeof(OBJ_VERT) };
		const UINT offsets[] = { 0 };
		ID3D11Buffer* const buffs[] = { cubeVertexBuffer.Get() };
		con->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, strides, offsets);
		con->IASetIndexBuffer(cubeIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		con->VSSetShader(vertexShader.Get(), nullptr, 0);
		con->PSSetShader(pixelShader.Get(), nullptr, 0);
		con->IASetInputLayout(vertexFormat.Get());
		con->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

		//con->PSSetSamplers(0, 1, &samplerState);

		// now we can draw
		con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// update subresource
		con->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &Send2Shader, sizeof(Send2Shader), 0);
		con->PSSetShaderResources(0, 1, srv.GetAddressOf());

		// Draw
		//con->DrawIndexed(_9cube_indexcount, 0, 0);

		//m.RotationYF(Send2Shader.world, rand(), Send2Shader.world);
		con->DrawIndexedInstanced(_9cube_indexcount, 5000, 0, 0, 0);

		// release temp handles
		view->Release();
		con->Release();
	}
	~Triangle()
	{
		// ComPtr will auto release so nothing to do here 
	}
};
