// minimalistic code to draw a single triangle, this is not part of the API.
// required for compiling shaders on the fly, consider pre-compiling instead
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include "cube.h"
#include "9cube.h"
#include "sphere.h"
#include "skybox.h"
#include "DDSTextureLoader.h"

// Shader Includes
#include "VertexShader.h"
#include "SkyBoxVertexShader.h"
#include "InstancedVertexShader.h"

#include "PixelShader.h"
#include "ReflectionShader.h"
#include "SkyBoxPixelShader.h"
#include "ColorOnlyPixelShader.h"

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

	GVECTORF CameraPos = { 5, 0, 0, 1 };
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

	// what we need at a minimum to draw a triangle
	Microsoft::WRL::ComPtr<ID3D11Buffer>		cube9VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		cube9IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		cubeVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		cubeIndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		skyboxVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		skyboxIndexBuffer;

	Microsoft::WRL::ComPtr<ID3D11Buffer>		constantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		pointLightConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		instConstantBuffer;

	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	skyBoxVertexShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	instVertexShader;

	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	skyBoxPixelShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	reflectionShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	colorOnlyPixelShader;

	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;

	Microsoft::WRL::ComPtr<ID3D11SamplerState>  samplerState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;

	// for texuring
	Microsoft::WRL::ComPtr<ID3D11Texture2D>		cubeTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	Microsoft::WRL::ComPtr<ID3D11Texture3D>		skyBoxTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skyBoxSRV;
	Microsoft::WRL::ComPtr<ID3D11Texture3D>		glassTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> glassSRV;

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
		GVECTORF at = { 0, 0, 0, 0 };
		GVECTORF up = { 0, 1, 0, 0 };
		m.LookAtLHF(eye, at, up, Send2Shader.view);
		m.LookAtLHF(eye, at, up, InstShader.view);

		//Lights
		//Point Light
		PointLight.pos = { 3.0f, 0.0f, 0.0f, 1.0f };
		PointLight.rgba = { 0.0f, 0.5f, 0.5f, 1.0f };
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
		CreateDDSTextureFromFile(creator, L"../metal.dds", (ID3D11Resource**)cubeTexture.GetAddressOf(), srv.GetAddressOf());
		//CreateDDSTextureFromFile(creator, L"../SkyboxOcean.dds", (ID3D11Resource**)skyBoxTexture.GetAddressOf(), skyBoxSRV.GetAddressOf());
		CreateDDSTextureFromFile(creator, L"../SunsetSkybox.dds", (ID3D11Resource**)skyBoxTexture.GetAddressOf(), skyBoxSRV.GetAddressOf());
		CreateDDSTextureFromFile(creator, L"../glass.dds", (ID3D11Resource**)glassTexture.GetAddressOf(), glassSRV.GetAddressOf());

	}void ControlCamera() {
		float key;

		cameraController.GetState(G_KEY_W, key);
		if (key > 0) {
			GVECTORF move = { 0.0f, 0.0f, -10.0f, 1.0f };
			m.TranslatelocalF(Send2Shader.view, move, Send2Shader.view);
		}

		cameraController.GetState(G_KEY_S, key);
		if (key > 0) {
			GVECTORF move = { 0.0f, 0.0f, 10.0f, 1.0f };
			m.TranslatelocalF(Send2Shader.view, move, Send2Shader.view);
		}

		cameraController.GetState(G_KEY_A, key);
		if (key > 0) {
			GVECTORF move = { 10.0f, 0.0f, 0.0f, 1.0f };
			m.TranslatelocalF(Send2Shader.view, move, Send2Shader.view);
		}

		cameraController.GetState(G_KEY_D, key);
		if (key > 0) {
			GVECTORF move = { -10.0f, 0.0f, 0.0f, 1.0f };
			m.TranslatelocalF(Send2Shader.view, move, Send2Shader.view);
		}

		cameraController.GetState(G_KEY_R, key);
		if (key > 0) {
			GVECTORF move = { 0.0f, -10.0f, 0.0f, 1.0f };
			m.TranslatelocalF(Send2Shader.view, move, Send2Shader.view);
		}

		cameraController.GetState(G_KEY_F, key);
		if (key > 0) {
			GVECTORF move = { 0.0f, 10.0f, 0.0f, 1.0f };
			m.TranslatelocalF(Send2Shader.view, move, Send2Shader.view);
		}

		float ar = 0;
		d3d.GetAspectRatio(ar);

		m.ProjectionDirectXLHF(G_PI / 2, ar, 0.1f, 100, Send2Shader.projection);
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

		ControlCamera();
		GVECTORF eye = CameraPos;
		GVECTORF at = { 0, 0, 0, 0 };
		GVECTORF up = { 0, 1, 0, 0 };
		m.LookAtLHF(eye, at, up, Send2Shader.view);
		m.LookAtLHF(eye, at, up, InstShader.view);
		PointLight.cameraPos = CameraPos;

		// modify world
		GMATRIXF temp = Send2Shader.world;

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

		//draw light
		con->OMSetBlendState(0, 0, 0xffffffff);
		GMATRIXF pLightMatrix = GIdentityMatrixF;
		pLightMatrix.row4 = PointLight.pos;
		m.RotationYF(pLightMatrix, 4 * -3.14f / 1000.0f, pLightMatrix);
		PointLight.pos = pLightMatrix.row4;
		con->PSSetShader(colorOnlyPixelShader.Get(), nullptr, 0);
		con->UpdateSubresource(pointLightConstantBuffer.Get(), 0, nullptr, &PointLight, 0, 0);
		con->PSSetConstantBuffers(0, 1, pointLightConstantBuffer.GetAddressOf());

		// Skybox
		con->PSSetShaderResources(1, 1, skyBoxSRV.GetAddressOf());
		con->IASetVertexBuffers(0, 1, skyboxVertexBuffer.GetAddressOf(), strides, offsets);
		con->IASetIndexBuffer(skyboxIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		con->VSSetShader(skyBoxVertexShader.Get(), nullptr, 0);
		con->PSSetShader(skyBoxPixelShader.Get(), nullptr, 0);
		con->IASetInputLayout(vertexFormat.Get());
		con->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
		con->RSSetState(rasterizerState.Get());

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
		con->IASetVertexBuffers(0, 1, cube9VertexBuffer.GetAddressOf(), strides, offsets);
		con->IASetIndexBuffer(cube9IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		con->VSSetShader(instVertexShader.Get(), nullptr, 0);
		con->PSSetShader(reflectionShader.Get(), nullptr, 0);
		con->VSSetConstantBuffers(0, 1, instConstantBuffer.GetAddressOf());

		GMATRIXF cubeWorld = GIdentityMatrixF;
		m.MultiplyMatrixF(cubeWorld, InstShader.world[0], cubeWorld);
		cubeWorld.row4 = { -2.5, 0, -5, 1 };
		m.RotationYF(cubeWorld, 3.14f / 1000.0f, cubeWorld);
		InstShader.world[0] = cubeWorld;

		GMATRIXF cubeWorld1 = GIdentityMatrixF;
		m.MultiplyMatrixF(cubeWorld1, InstShader.world[1], cubeWorld1);
		cubeWorld1.row4 = { -2.5, 0, 5, 1 };
		m.RotationYF(cubeWorld1, -3.14f / 1000.0f, cubeWorld1);
		InstShader.world[1] = cubeWorld1;

		GMATRIXF cubeWorld2 = GIdentityMatrixF;
		cubeWorld2.row4 = { 0, -5, 0, 1 };
		InstShader.world[2] = cubeWorld2;

		// update subresource
		con->UpdateSubresource(instConstantBuffer.Get(), 0, nullptr, &InstShader, 0, 0);
		con->PSSetShaderResources(0, 1, srv.GetAddressOf());

		// draw object
		con->DrawIndexedInstanced(_9cube_indexcount, 3, 0, 0, 0);;

		con->IASetVertexBuffers(0, 1, cubeVertexBuffer.GetAddressOf(), strides, offsets);
		con->IASetIndexBuffer(cubeIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		con->VSSetShader(vertexShader.Get(), nullptr, 0);
		con->PSSetShader(colorOnlyPixelShader.Get(), nullptr, 0);
		con->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

		GMATRIXF pointLightCubeWorld = GIdentityMatrixF;
		scale = { 0.5f, 0.5f, 0.5f, 1 };
		m.ScalingF(pointLightCubeWorld, scale, pointLightCubeWorld);
		pointLightCubeWorld.row4 = PointLight.pos;
		Send2Shader.world = pointLightCubeWorld;

		con->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &Send2Shader, 0, 0);
		con->PSSetShaderResources(0, 1, srv.GetAddressOf());

		con->DrawIndexed(cube_indexcount, 0, 0);

		// Transparent Cube

		float blendFactor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		con->OMSetBlendState(blendState.Get(), blendFactor, 0xffffffff);

		con->PSSetShader(pixelShader.Get(), nullptr, 0);

		GMATRIXF transparentCubeWorld = GIdentityMatrixF;
		scale = { 1.0f, 1.0f, 1.0f, 1 };
		m.ScalingF(transparentCubeWorld, scale, transparentCubeWorld);
		m.RotationZF(transparentCubeWorld, -15 * 3.14 / 180, transparentCubeWorld);
		transparentCubeWorld.row4 = { 0, 0, -2, 1 };
		Send2Shader.world = transparentCubeWorld;

		con->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &Send2Shader, 0, 0);
		con->PSSetShaderResources(0, 1, glassSRV.GetAddressOf());

		con->DrawIndexed(cube_indexcount, 0, 0);

		// release temp handles
		view->Release();
		con->Release();
	}



	~Triangle()
	{
		// ComPtr will auto release so nothing to do here 
	}
};
