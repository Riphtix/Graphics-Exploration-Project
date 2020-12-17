// minimalistic code to draw a single triangle, this is not part of the API.
// required for compiling shaders on the fly, consider pre-compiling instead
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include "cube.h"
#include "9cube.h"
#include "sphere.h"
#include "skybox.h"
#include "DwarfArmor.h"
#include "DwarfAxe.h"
#include "DwarfBody.h"
#include "DwarfLeather.h"
#include "DwarfShirt.h"
#include "DDSTextureLoader.h"

// Shader Includes
#include "VertexShader.h"
#include "SkyBoxVertexShader.h"
#include "InstancedVertexShader.h"

#include "PixelShader.h"
#include "ReflectionShader.h"
#include "SkyBoxPixelShader.h"
#include "ColorOnlyPixelShader.h"
#include "SpecPixelShader.h"

using namespace GW::MATH;

// Creation, Rendering & Cleanup
class Triangle
{
	// math things
	struct SHADER_VARIABLES {
		GMATRIXF world = GIdentityMatrixF;
		GMATRIXF view = GIdentityMatrixF;
		GMATRIXF projection = GIdentityMatrixF;
	}Send2Shader;

	GVECTORF CameraPos = { -1, 1.25, 2, 1 };
	D3D_DRIVER_TYPE g_driverType = D3D_DRIVER_TYPE_NULL;

	struct P_Light {
		GVECTORF pos;
		GVECTORF rgba;
		GVECTORF radius;
		GVECTORF cameraPos;
	}PointLight;

	struct Instanced_Shader_Variables {
		GMATRIXF world[3] = { GIdentityMatrixF, GIdentityMatrixF, GIdentityMatrixF };
		GMATRIXF view = GIdentityMatrixF;
		GMATRIXF projection = GIdentityMatrixF;
	}InstShader;

	// handle for math lib
	GMatrix m;
	GVector v;

	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	GW::INPUT::GInput cameraController;

	// Models
	Microsoft::WRL::ComPtr<ID3D11Buffer>		cube9VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		cube9IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		cubeVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		cubeIndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		skyboxVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		skyboxIndexBuffer;

	Microsoft::WRL::ComPtr<ID3D11Buffer>		dwarfBodyVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		dwarfBodyIndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		dwarfShirtVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		dwarfShirtIndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		dwarfLeatherVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		dwarfLeatherIndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		dwarfArmorVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		dwarfArmorIndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		dwarfAxeVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		dwarfAxeIndexBuffer;

	// Constant Buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer>		constantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		pointLightConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		instConstantBuffer;

	// Shaders
	// Vertex
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	skyBoxVertexShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	instVertexShader;

	// Pixel
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	skyBoxPixelShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	reflectionShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	colorOnlyPixelShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	specPixelShader;

	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;

	// States
	Microsoft::WRL::ComPtr<ID3D11SamplerState>  samplerState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;

	// Textures
	Microsoft::WRL::ComPtr<ID3D11Texture2D>		metalTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalSRV;
	Microsoft::WRL::ComPtr<ID3D11Texture3D>		glassTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> glassSRV;

	Microsoft::WRL::ComPtr<ID3D11Texture2D>		dwarfBodyTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> dwarfBodySRV;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>		dwarfBodySpecMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> dwarfBodySpecSRV;

	Microsoft::WRL::ComPtr<ID3D11Texture2D>		dwarfShirtTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> dwarfShirtSRV;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>		dwarfShirtSpecMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> dwarfShirtSpecSRV;

	Microsoft::WRL::ComPtr<ID3D11Texture2D>		dwarfLeatherTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> dwarfLeatherSRV;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>		dwarfLeatherSpecMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> dwarfLeatherSpecSRV;

	Microsoft::WRL::ComPtr<ID3D11Texture2D>		dwarfArmorTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> dwarfArmorSRV;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>		dwarfArmorSpecMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> dwarfArmorSpecSRV;

	Microsoft::WRL::ComPtr<ID3D11Texture3D>		skyBoxTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skyBoxSRV;

public:
	Triangle(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		win = _win;
		d3d = _d3d;
		ID3D11Device* creator;
		d3d.GetDevice((void**)&creator);

		cameraController.Create(_win);

		D3D11_SUBRESOURCE_DATA cube9BData = { _9cube_data, 0, 0 };
		CD3D11_BUFFER_DESC cube9BDesc(sizeof(_9cube_data), D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&cube9BDesc, &cube9BData, cube9VertexBuffer.GetAddressOf());

		D3D11_SUBRESOURCE_DATA cube9IData = { _9cube_indicies, 0, 0 };
		CD3D11_BUFFER_DESC cube9IDesc(sizeof(_9cube_indicies), D3D11_BIND_INDEX_BUFFER);
		creator->CreateBuffer(&cube9IDesc, &cube9IData, cube9IndexBuffer.GetAddressOf());

		D3D11_SUBRESOURCE_DATA cubeBData = { cube_data, 0, 0 };
		CD3D11_BUFFER_DESC cubeBDesc(sizeof(cube_data), D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&cubeBDesc, &cubeBData, cubeVertexBuffer.GetAddressOf());

		D3D11_SUBRESOURCE_DATA cubeIData = { _9cube_indicies, 0, 0 };
		CD3D11_BUFFER_DESC cubeIDesc(sizeof(_9cube_indicies), D3D11_BIND_INDEX_BUFFER);
		creator->CreateBuffer(&cubeIDesc, &cubeIData, cubeIndexBuffer.GetAddressOf());

		D3D11_SUBRESOURCE_DATA skyboxBData = { skybox_data, 0, 0 };
		CD3D11_BUFFER_DESC skyboxBDesc(sizeof(skybox_data), D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&skyboxBDesc, &skyboxBData, skyboxVertexBuffer.GetAddressOf());

		D3D11_SUBRESOURCE_DATA skyboxIData = { skybox_indicies, 0, 0 };
		CD3D11_BUFFER_DESC skyboxIDesc(sizeof(skybox_indicies), D3D11_BIND_INDEX_BUFFER);
		creator->CreateBuffer(&skyboxIDesc, &skyboxIData, skyboxIndexBuffer.GetAddressOf());

		D3D11_SUBRESOURCE_DATA dwarfBodyBData = { DwarfBody_data, 0, 0 };
		CD3D11_BUFFER_DESC dwarfBodyBDesc(sizeof(DwarfBody_data), D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&dwarfBodyBDesc, &dwarfBodyBData, dwarfBodyVertexBuffer.GetAddressOf());

		D3D11_SUBRESOURCE_DATA dwarfBodyIData = { DwarfBody_indicies, 0, 0 };
		CD3D11_BUFFER_DESC dwarfBodyIDesc(sizeof(DwarfBody_indicies), D3D11_BIND_INDEX_BUFFER);
		creator->CreateBuffer(&dwarfBodyIDesc, &dwarfBodyIData, dwarfBodyIndexBuffer.GetAddressOf());

		D3D11_SUBRESOURCE_DATA dwarfShirtBData = { DwarfShirt_data, 0, 0 };
		CD3D11_BUFFER_DESC dwarfShirtBDesc(sizeof(DwarfShirt_data), D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&dwarfShirtBDesc, &dwarfShirtBData, dwarfShirtVertexBuffer.GetAddressOf());

		D3D11_SUBRESOURCE_DATA dwarfShirtIData = { DwarfShirt_indicies, 0, 0 };
		CD3D11_BUFFER_DESC dwarfShirtIDesc(sizeof(DwarfShirt_indicies), D3D11_BIND_INDEX_BUFFER);
		creator->CreateBuffer(&dwarfShirtIDesc, &dwarfShirtIData, dwarfShirtIndexBuffer.GetAddressOf());

		D3D11_SUBRESOURCE_DATA dwarfLeatherBData = { DwarfLeather_data, 0, 0 };
		CD3D11_BUFFER_DESC dwarfLeatherBDesc(sizeof(DwarfLeather_data), D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&dwarfLeatherBDesc, &dwarfLeatherBData, dwarfLeatherVertexBuffer.GetAddressOf());

		D3D11_SUBRESOURCE_DATA dwarfLeatherIData = { DwarfLeather_indicies, 0, 0 };
		CD3D11_BUFFER_DESC dwarfLeatherIDesc(sizeof(DwarfLeather_indicies), D3D11_BIND_INDEX_BUFFER);
		creator->CreateBuffer(&dwarfLeatherIDesc, &dwarfLeatherIData, dwarfLeatherIndexBuffer.GetAddressOf());

		D3D11_SUBRESOURCE_DATA dwarfArmorBData = { DwarfArmor_data, 0, 0 };
		CD3D11_BUFFER_DESC dwarfArmorBDesc(sizeof(DwarfArmor_data), D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&dwarfArmorBDesc, &dwarfArmorBData, dwarfArmorVertexBuffer.GetAddressOf());

		D3D11_SUBRESOURCE_DATA dwarfArmorIData = { DwarfArmor_indicies, 0, 0 };
		CD3D11_BUFFER_DESC dwarfArmorIDesc(sizeof(DwarfArmor_indicies), D3D11_BIND_INDEX_BUFFER);
		creator->CreateBuffer(&dwarfArmorIDesc, &dwarfArmorIData, dwarfArmorIndexBuffer.GetAddressOf());

		D3D11_SUBRESOURCE_DATA dwarfAxeBData = { DwarfAxe_data, 0, 0 };
		CD3D11_BUFFER_DESC dwarfAxeBDesc(sizeof(DwarfAxe_data), D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&dwarfAxeBDesc, &dwarfAxeBData, dwarfAxeVertexBuffer.GetAddressOf());

		D3D11_SUBRESOURCE_DATA dwarfAxeIData = { DwarfAxe_indicies, 0, 0 };
		CD3D11_BUFFER_DESC dwarfAxeIDesc(sizeof(DwarfAxe_indicies), D3D11_BIND_INDEX_BUFFER);
		creator->CreateBuffer(&dwarfAxeIDesc, &dwarfAxeIData, dwarfAxeIndexBuffer.GetAddressOf());

		D3D11_BUFFER_DESC pLightDesc;
		ZeroMemory(&pLightDesc, sizeof(pLightDesc));
		pLightDesc.ByteWidth = sizeof(P_Light);
		pLightDesc.Usage = D3D11_USAGE_DEFAULT;
		pLightDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		pLightDesc.CPUAccessFlags = 0;
		pLightDesc.MiscFlags = 0;
		pLightDesc.StructureByteStride = 0;
		creator->CreateBuffer(&pLightDesc, nullptr, constantBuffer.GetAddressOf());

		CD3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
		creator->CreateSamplerState(&samplerDesc, &samplerState);

		// Create Vertex Shader
		UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		compilerFlags |= D3DCOMPILE_DEBUG;
#endif

		// create vertex shaders
		creator->CreateVertexShader(VertexShader, sizeof(VertexShader), nullptr, vertexShader.GetAddressOf());
		creator->CreateVertexShader(SkyBoxVertexShader, sizeof(SkyBoxVertexShader), nullptr, skyBoxVertexShader.GetAddressOf());
		creator->CreateVertexShader(InstancedVertexShader, sizeof(InstancedVertexShader), nullptr, instVertexShader.GetAddressOf());

		// Create Pixel Shaders
		creator->CreatePixelShader(PixelShader, sizeof(PixelShader), nullptr, pixelShader.GetAddressOf());
		creator->CreatePixelShader(SkyBoxPixelShader, sizeof(SkyBoxPixelShader), nullptr, skyBoxPixelShader.GetAddressOf());
		creator->CreatePixelShader(ReflectionShader, sizeof(ReflectionShader), nullptr, reflectionShader.GetAddressOf());
		creator->CreatePixelShader(ColorOnlyPixelShader, sizeof(ColorOnlyPixelShader), nullptr, colorOnlyPixelShader.GetAddressOf());
		creator->CreatePixelShader(SpecPixelShader, sizeof(SpecPixelShader), nullptr, specPixelShader.GetAddressOf());

		// Create Input Layout
		D3D11_INPUT_ELEMENT_DESC format[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		creator->CreateInputLayout(format, ARRAYSIZE(format), VertexShader, sizeof(VertexShader), vertexFormat.GetAddressOf());
		creator->CreateInputLayout(format, ARRAYSIZE(format), SkyBoxVertexShader, sizeof(SkyBoxVertexShader), vertexFormat.GetAddressOf());
		creator->CreateInputLayout(format, ARRAYSIZE(format), InstancedVertexShader, sizeof(InstancedVertexShader), vertexFormat.GetAddressOf());

		CD3D11_RASTERIZER_DESC rasterizerDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
		rasterizerDesc.CullMode = D3D11_CULL_NONE;

		creator->CreateRasterizerState(&rasterizerDesc, &rasterizerState);

		D3D11_DEPTH_STENCIL_DESC depthDesc;
		ZeroMemory(&depthDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
		depthDesc.DepthEnable = true;
		depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		creator->CreateDepthStencilState(&depthDesc, &depthState);

		CD3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());

		D3D11_RENDER_TARGET_BLEND_DESC rtBlendDesc;
		ZeroMemory(&rtBlendDesc, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
		rtBlendDesc.BlendEnable = true;
		rtBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		rtBlendDesc.DestBlend = D3D11_BLEND_BLEND_FACTOR;
		rtBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
		rtBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
		rtBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
		rtBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		rtBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		blendDesc.AlphaToCoverageEnable = true;
		blendDesc.RenderTarget[0] = rtBlendDesc;
		creator->CreateBlendState(&blendDesc, blendState.GetAddressOf());


		// free temporary handle
		creator->Release();

		// math setup here
		m.Create();

		GVECTORF eye = CameraPos;
		GVECTORF at = { 0, 1, 0, 0 };
		GVECTORF up = { 0, 1, 0, 0 };
		m.LookAtLHF(eye, at, up, Send2Shader.view);
		m.LookAtLHF(eye, at, up, InstShader.view);

		//Lights
		//Point Light
		PointLight.pos = { 3.0f, 0.0f, 0.0f, 1.0f };
		PointLight.rgba = { 0.0f, 0.0f, 0.0f, 1.0f };
		PointLight.radius = { 10.0f, 0.0f, 0.0f, 0.0f };
		PointLight.cameraPos = CameraPos;


		D3D11_SUBRESOURCE_DATA cData = { &Send2Shader, 0, 0 };
		CD3D11_BUFFER_DESC cDesc(sizeof(Send2Shader), D3D11_BIND_CONSTANT_BUFFER);
		creator->CreateBuffer(&cDesc, &cData, constantBuffer.GetAddressOf());

		D3D11_SUBRESOURCE_DATA pLightCData = { &Send2Shader, 0, 0 };
		CD3D11_BUFFER_DESC pLightCDesc(sizeof(Send2Shader), D3D11_BIND_CONSTANT_BUFFER);
		creator->CreateBuffer(&pLightCDesc, &pLightCData, pointLightConstantBuffer.GetAddressOf());

		D3D11_SUBRESOURCE_DATA instCData = { &InstShader, 0, 0 };
		CD3D11_BUFFER_DESC instCDesc(sizeof(InstShader), D3D11_BIND_CONSTANT_BUFFER);
		creator->CreateBuffer(&instCDesc, &instCData, instConstantBuffer.GetAddressOf());

		// load texture into VRAM
		//CreateDDSTextureFromFile(creator, L"../SkyboxOcean.dds", (ID3D11Resource**)skyBoxTexture.GetAddressOf(), skyBoxSRV.GetAddressOf());
		//CreateDDSTextureFromFile(creator, L"../SunsetSkybox.dds", (ID3D11Resource**)skyBoxTexture.GetAddressOf(), skyBoxSRV.GetAddressOf());
		CreateDDSTextureFromFile(creator, L"../LostValley.dds", (ID3D11Resource**)skyBoxTexture.GetAddressOf(), skyBoxSRV.GetAddressOf());

		CreateDDSTextureFromFile(creator, L"../glass.dds", (ID3D11Resource**)glassTexture.GetAddressOf(), glassSRV.GetAddressOf());
		CreateDDSTextureFromFile(creator, L"../metal.dds", (ID3D11Resource**)metalTexture.GetAddressOf(), metalSRV.GetAddressOf());

		CreateDDSTextureFromFile(creator, L"../BodyTexture.dds", (ID3D11Resource**)dwarfBodyTexture.GetAddressOf(), dwarfBodySRV.GetAddressOf());
		CreateDDSTextureFromFile(creator, L"../BodySpec.dds", (ID3D11Resource**)dwarfBodySpecMap.GetAddressOf(), dwarfBodySpecSRV.GetAddressOf());
		CreateDDSTextureFromFile(creator, L"../ShirtTexture.dds", (ID3D11Resource**)dwarfShirtTexture.GetAddressOf(), dwarfShirtSRV.GetAddressOf());
		CreateDDSTextureFromFile(creator, L"../ShirtSpec.dds", (ID3D11Resource**)dwarfShirtSpecMap.GetAddressOf(), dwarfShirtSpecSRV.GetAddressOf());
		CreateDDSTextureFromFile(creator, L"../LeatherTexture.dds", (ID3D11Resource**)dwarfLeatherTexture.GetAddressOf(), dwarfLeatherSRV.GetAddressOf());
		CreateDDSTextureFromFile(creator, L"../LeatherSpec.dds", (ID3D11Resource**)dwarfLeatherSpecMap.GetAddressOf(), dwarfLeatherSpecSRV.GetAddressOf());
		CreateDDSTextureFromFile(creator, L"../ArmorTexture.dds", (ID3D11Resource**)dwarfArmorTexture.GetAddressOf(), dwarfArmorSRV.GetAddressOf());
		CreateDDSTextureFromFile(creator, L"../ArmorSpec.dds", (ID3D11Resource**)dwarfArmorSpecMap.GetAddressOf(), dwarfArmorSpecSRV.GetAddressOf());
	}

	float Deg2Rad(float deg) {
		return deg * 3.14f / 180.0f;
	}

	void Render()
	{

		// grab the context & render target
		ID3D11DeviceContext* con;
		ID3D11RenderTargetView* view;
		ID3D11DepthStencilView* depth;
		d3d.GetImmediateContext((void**)&con);
		d3d.GetRenderTargetView((void**)&view);
		d3d.GetDepthStencilView((void**)&depth);

		// modify world
		m.RotationYF(Send2Shader.world, 1000.0f, Send2Shader.world);

		// aspect ratio
		float ar;
		d3d.GetAspectRatio(ar);
		m.ProjectionDirectXLHF(G_PI / 2, ar, 0.01f, 1000.0f, Send2Shader.projection);
		m.ProjectionDirectXLHF(G_PI / 2, ar, 0.01f, 1000.0f, InstShader.projection);

		// setup the pipeline
		ID3D11RenderTargetView* const views[] = { view };
		con->OMSetRenderTargets(ARRAYSIZE(views), views, depth);
		const UINT strides[] = { sizeof(OBJ_VERT) };
		const UINT offsets[] = { 0 };

		con->PSSetSamplers(0, 1, &samplerState);

		// now we can draw
		con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		con->OMSetBlendState(0, 0, 0xffffffff);

		// Skybox
		con->PSSetShaderResources(1, 1, skyBoxSRV.GetAddressOf());
		con->IASetVertexBuffers(0, 1, skyboxVertexBuffer.GetAddressOf(), strides, offsets);
		con->IASetIndexBuffer(skyboxIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		con->VSSetShader(skyBoxVertexShader.Get(), nullptr, 0);
		con->PSSetShader(skyBoxPixelShader.Get(), nullptr, 0);
		con->IASetInputLayout(vertexFormat.Get());
		con->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
		//con->RSSetState(rasterizerState.Get());

		con->OMSetDepthStencilState(depthState.Get(), 0);

		GMATRIXF skyboxWorld = GIdentityMatrixF;
		skyboxWorld.row4 = CameraPos;
		Send2Shader.world = skyboxWorld;

		// update subresource
		con->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &Send2Shader, 0, 0);

		// draw skybox
		con->DrawIndexed(skybox_indexcount, 0, 0);

		con->ClearDepthStencilView(depth, D3D11_CLEAR_DEPTH, 1, 255);

		GVECTORF scale = { 1.0f, 1.0f, 1.0f, 1 };

		// Instanced Objects

		// Uninstanced objects

		//body
		con->IASetVertexBuffers(0, 1, dwarfBodyVertexBuffer.GetAddressOf(), strides, offsets);
		con->IASetIndexBuffer(dwarfBodyIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		con->VSSetShader(vertexShader.Get(), nullptr, 0);
		con->PSSetShader(specPixelShader.Get(), nullptr, 0);
		con->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

		GMATRIXF dwarfBody = Send2Shader.world;
		m.RotationYF(dwarfBody, Deg2Rad(180), dwarfBody);
		dwarfBody.row4 = { -1, 0, 0, 1 };
		Send2Shader.world = dwarfBody;

		ID3D11ShaderResourceView** textureArray[2] = { dwarfBodySRV.GetAddressOf(), dwarfBodySpecSRV.GetAddressOf() };

		con->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &Send2Shader, 0, 0);
		con->PSSetShaderResources(0, 1, *textureArray);

		con->DrawIndexed(DwarfBody_indexcount, 0, 0);

		//shirt
		con->IASetVertexBuffers(0, 1, dwarfShirtVertexBuffer.GetAddressOf(), strides, offsets);
		con->IASetIndexBuffer(dwarfShirtIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		GMATRIXF dwarfShirt = dwarfBody;
		Send2Shader.world = dwarfShirt;

		textureArray[0] = dwarfShirtSRV.GetAddressOf();
		textureArray[1] = dwarfShirtSpecSRV.GetAddressOf();

		con->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &Send2Shader, 0, 0);
		con->PSSetShaderResources(0, 1, *textureArray);

		con->DrawIndexed(DwarfShirt_indexcount, 0, 0);

		//leather
		con->IASetVertexBuffers(0, 1, dwarfLeatherVertexBuffer.GetAddressOf(), strides, offsets);
		con->IASetIndexBuffer(dwarfLeatherIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		GMATRIXF dwarfLeather = dwarfBody;
		Send2Shader.world = dwarfLeather;

		textureArray[0] = dwarfLeatherSRV.GetAddressOf();
		textureArray[1] = dwarfLeatherSpecSRV.GetAddressOf();

		con->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &Send2Shader, 0, 0);
		con->PSSetShaderResources(0, 1, *textureArray);

		con->DrawIndexed(DwarfLeather_indexcount, 0, 0);

		//armor
		con->IASetVertexBuffers(0, 1, dwarfArmorVertexBuffer.GetAddressOf(), strides, offsets);
		con->IASetIndexBuffer(dwarfArmorIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		GMATRIXF dwarfArmor = dwarfBody;
		Send2Shader.world = dwarfArmor;

		textureArray[0] = dwarfArmorSRV.GetAddressOf();
		textureArray[1] = dwarfArmorSpecSRV.GetAddressOf();

		con->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &Send2Shader, 0, 0);
		con->PSSetShaderResources(0, 1, *textureArray);

		con->DrawIndexed(DwarfArmor_indexcount, 0, 0);

		//axe
		con->IASetVertexBuffers(0, 1, dwarfAxeVertexBuffer.GetAddressOf(), strides, offsets);
		con->IASetIndexBuffer(dwarfAxeIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		con->PSSetShader(reflectionShader.Get(), nullptr, 0);

		GMATRIXF dwarfAxe = dwarfBody;
		//m.MultiplyMatrixF(GIdentityMatrixF, Send2Shader.world, dwarfAxe);
		dwarfAxe.row4 = { 1.5f, 0.0f, -2.5f, 1.0f };

		m.RotationYF(dwarfAxe, Deg2Rad(90), dwarfAxe);
		m.RotationXF(dwarfAxe, Deg2Rad(170), dwarfAxe);
		m.RotationZF(dwarfAxe, Deg2Rad(0), dwarfAxe);
		scale = { 0.3f, 0.3f, 0.3f, 1 };
		m.ScalingF(dwarfAxe, scale, dwarfAxe);
		Send2Shader.world = dwarfAxe;

		con->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &Send2Shader, 0, 0);
		con->PSSetShaderResources(0, 1, metalSRV.GetAddressOf());

		con->DrawIndexed(DwarfAxe_indexcount, 0, 0);

		// release temp handles
		view->Release();
		con->Release();
	}

	~Triangle()
	{
		// ComPtr will auto release so nothing to do here 
	}
};
