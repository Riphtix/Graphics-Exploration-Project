// minimalistic code to draw a single triangle, this is not part of the API.
// required for compiling shaders on the fly, consider pre-compiling instead
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
// Simple Vertex Shader
const char* vertexShaderSource = R"(
// an ultra simple hlsl vertex shader
float4 main(float2 inputVertex : POSITION) : SV_POSITION
{
	return float4(inputVertex, 0, 1);
}
)";
// Simple Pixel Shader
const char* pixelShaderSource = R"(
// an ultra simple hlsl pixel shader
float4 main() : SV_TARGET 
{	
	return float4(0.25f,0.0f,1.0f,0); 
}
)";
// Creation, Rendering & Cleanup
class Triangle
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	// what we need at a minimum to draw a triangle
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;
public:
	Triangle(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		win = _win;
		d3d = _d3d;
		ID3D11Device* creator;
		d3d.GetDevice((void**)&creator);
		// Create Vertex Buffer
		float verts[] = {
			   0,   0.5f,
			 0.5f, -0.5f,
			-0.5f, -0.5f
		};
		D3D11_SUBRESOURCE_DATA bData = { verts, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeof(verts), D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&bDesc, &bData, vertexBuffer.GetAddressOf());
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
		// Create Pixel Shader
		Microsoft::WRL::ComPtr<ID3DBlob> psBlob; errors.Reset();
		if (SUCCEEDED(D3DCompile(pixelShaderSource, strlen(pixelShaderSource),
			nullptr, nullptr, nullptr, "main", "ps_4_0", compilerFlags, 0, 
			psBlob.GetAddressOf(), errors.GetAddressOf())))
		{
			creator->CreatePixelShader(psBlob->GetBufferPointer(),
				psBlob->GetBufferSize(), nullptr, pixelShader.GetAddressOf());
		}
		else
			std::cout << (char*)errors->GetBufferPointer() << std::endl;
		// Create Input Layout
		D3D11_INPUT_ELEMENT_DESC format[] = {
			{ 
				"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 
			}
		};
		creator->CreateInputLayout(format, ARRAYSIZE(format), 
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), 
			vertexFormat.GetAddressOf());
		// free temporary handle
		creator->Release();
	}
	void Render()
	{
		// grab the context & render target
		ID3D11DeviceContext* con;
		ID3D11RenderTargetView* view;
		d3d.GetImmediateContext((void**)&con);
		d3d.GetRenderTargetView((void**)&view);
		// setup the pipeline
		ID3D11RenderTargetView *const views[] = { view };
		con->OMSetRenderTargets(ARRAYSIZE(views), views, nullptr);
		const UINT strides[] = { sizeof(float) * 2 };
		const UINT offsets[] = { 0 };
		ID3D11Buffer* const buffs[] = { vertexBuffer.Get() };
		con->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, strides, offsets);
		con->VSSetShader(vertexShader.Get(), nullptr, 0);
		con->PSSetShader(pixelShader.Get(), nullptr, 0);
		con->IASetInputLayout(vertexFormat.Get());
		// now we can draw
		con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		con->Draw(3, 0);
		// release temp handles
		view->Release();
		con->Release();
	}
	~Triangle()
	{
		// ComPtr will auto release so nothing to do here 
	}
};
