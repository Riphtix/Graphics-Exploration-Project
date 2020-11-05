// Main.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Main.h"

// Include directx11
#include <d3d11.h>
#include <iostream>
#pragma comment(lib, "d3d11.lib")

#include <DirectXMath.h>
using namespace DirectX;

#include "VertexShader.csh"
#include "PixelShader.csh"

#include "MeshVertexShader.csh"

#include "Sword.h"

// Defines
#define DEBUG true
#define MAX_LOADSTRING 100

// Teal Hex		#119da7
// Teal RGB		17, 157, 167
// Teal Ratio	0.0667, 0.6157, 0.6549
// Orange Hex	#f5a61d
// Orange RGB	245, 166, 29
// Orange Ratio	0.9608, 0.651, 0.1137

// For init
ID3D11Device* dev;
IDXGISwapChain* swap;
ID3D11DeviceContext* context;

// For drawing
ID3D11RenderTargetView* rtv;
D3D11_VIEWPORT viewPort;
float aspectRatio = 1;

struct Vertex {
	float xyzw[4];
	float rgba[4];
};

unsigned int numVerts = 0;

ID3D11Buffer* vBuff;
ID3D11InputLayout* vLayout;
ID3D11VertexShader* vShader; // HLSL
ID3D11PixelShader* pShader;  // HLSL

// Shader Variables
ID3D11Buffer* cBuff;

// Mesh Data
ID3D11Buffer* vBuffMesh; // Vertex buffer
ID3D11Buffer* iBuffMesh; // Index buffer

// Mesh vertex shader
ID3D11VertexShader* vMeshShader;
ID3D11InputLayout* vMeshLayout;

// Z buffer for depth sorting
ID3D11Texture2D* zBuffer;
ID3D11DepthStencilView* zBufferView;

// Math stuff
struct WVP {
	XMFLOAT4X4 wMatrix;
	XMFLOAT4X4 vMatrix;
	XMFLOAT4X4 pMatrix;
}Matricies;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_CODE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CODE));

	MSG msg;

	// Main game loop:
	while (true)
	{
		// Handle windows messages during game loop
		PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
			break;

		// Render things
		float bgColor[] = { 0.0667, 0.6157, 0.6549, 1 };
		context->ClearRenderTargetView(rtv, bgColor);
		context->ClearDepthStencilView(zBufferView, D3D11_CLEAR_DEPTH, 1, 0);

		// Setup the pipeline

		// Output merger
		ID3D11RenderTargetView* tempRTV[] = { rtv };
		context->OMSetRenderTargets(1, tempRTV, zBufferView);

		// Rasterizer
		context->RSSetViewports(1, &viewPort);

		// Input Assembler
		context->IASetInputLayout(vLayout);

		UINT strides[] = { sizeof(Vertex) };
		UINT offsets[] = { 0 };
		ID3D11Buffer* tempVB[] = { vBuff };
		context->IASetVertexBuffers(0, 1, tempVB, strides, offsets);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Vertex Shader Stage
		context->VSSetShader(vShader, 0, 0);
		// Pixel Shader Stage
		context->PSSetShader(pShader, 0, 0);

		// Try and make triangle 3D
		// Make into a pyramid (more verts) - DONE
		// Make a world, view & projection matrix
		// World
		static float rot = 0;
		rot += 0.025f;
		XMMATRIX temp = XMMatrixIdentity();
		temp = XMMatrixTranslation(0, 0, 2);
		XMMATRIX temp2 = XMMatrixRotationY(rot);
		temp = XMMatrixMultiply(temp2, temp);
		XMStoreFloat4x4(&Matricies.wMatrix, temp);

		// View
		temp = XMMatrixLookAtLH({ 2, 10, -5 }, { 0, 0, 0 }, { 0, 1, 0 });
		XMStoreFloat4x4(&Matricies.vMatrix, temp);

		// Projection
		temp = XMMatrixPerspectiveFovLH(3.14f / 2.0f, aspectRatio, 0.1f, 1000);
		XMStoreFloat4x4(&Matricies.pMatrix, temp);

		// Upload those matricies to the video card
			// Create and update a constant buffer (move variables from c++ to shaders)
		D3D11_MAPPED_SUBRESOURCE gpuBuffer;
		context->Map(cBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
		memcpy(gpuBuffer.pData, &Matricies, sizeof(WVP));
		context->Unmap(cBuff, 0);

		// Apply matrix math in Vertex Shader - DONE
		// Connect constant buffer to the pipeline - DONE
		ID3D11Buffer* constants[] = { cBuff };
		context->VSSetConstantBuffers(0, 1, constants);

		// Draw
		context->Draw(numVerts, 0);

		// immediate context

		// Get a more complex pre-made mesh (FBX, OBJ, custom header) - DONE
		// load it into the card - DONE
		// make sure our shaders can process it - DONE
		// place it somewhere else in the environment

		// Set up mesh pipeline
		UINT mesh_strides[] = { sizeof(_OBJ_VERT_) };
		UINT mesh_offsets[] = { 0 };
		ID3D11Buffer* meshVB[] = { vBuffMesh };
		context->IASetVertexBuffers(0, 1, meshVB, mesh_strides, mesh_offsets);
		context->IASetIndexBuffer(iBuffMesh, DXGI_FORMAT_R32_UINT, 0);
		context->VSSetShader(vMeshShader, 0, 0);
		context->IASetInputLayout(vMeshLayout);

		// Modify world matrix before drawing next thing
		temp = XMMatrixIdentity();
		XMStoreFloat4x4(&Matricies.wMatrix, temp);

		// Send changes to card
		context->Map(cBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
		memcpy(gpuBuffer.pData, &Matricies, sizeof(WVP));
		context->Unmap(cBuff, 0);

		// draw it
		context->DrawIndexed(286733, 0, 0);

		swap->Present(1, 0);
	}

	// Release D3D11 Interfaces
	rtv->Release();
	vBuff->Release();
	cBuff->Release();
	vBuffMesh->Release();
	iBuffMesh->Release();
	vLayout->Release();
	vMeshLayout->Release();
	vShader->Release();
	pShader->Release();
	swap->Release();
	context->Release();
	dev->Release();

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CODE));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CODE);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	RECT windowRef;
	GetClientRect(hWnd, &windowRef);

	// Attach D3D11 to our window
	D3D_FEATURE_LEVEL dx11 = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC swapDesc;

	// Init swap descriptor
	ZeroMemory(&swapDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapDesc.BufferCount = 1;
	swapDesc.OutputWindow = hWnd;
	swapDesc.Windowed = true;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.Width = windowRef.right - windowRef.left;
	swapDesc.BufferDesc.Height = windowRef.bottom - windowRef.top;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.SampleDesc.Count = 1;

	aspectRatio = swapDesc.BufferDesc.Width / swapDesc.BufferDesc.Height;

	// Create Device and Swap Chain
	if (DEBUG) {
		D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG,
			&dx11, 1, D3D11_SDK_VERSION, &swapDesc, &swap, &dev, NULL, &context);
	}
	else {
		D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL,
			&dx11, 1, D3D11_SDK_VERSION, &swapDesc, &swap, &dev, NULL, &context);
	}

	ID3D11Resource* backBuffer;
	swap->GetBuffer(0, __uuidof(backBuffer), (void**)&backBuffer);
	dev->CreateRenderTargetView(backBuffer, NULL, &rtv);
	backBuffer->Release();

	viewPort.Width = swapDesc.BufferDesc.Width;
	viewPort.Height = swapDesc.BufferDesc.Height;
	viewPort.TopLeftX = viewPort.TopLeftY = 0;
	viewPort.MinDepth = 0;
	viewPort.MaxDepth = 1;

	Vertex triangle[] = // NDC Normalized Device Coordinates
	{ // xyzw, rgba
		//front
		{{0, 1.0f, 0, 1},{0.9608, 0.651, 0.1137, 1}}, // Orange
		{{0.25f, -0.25f, -0.25f, 1},{0.0627, 0.7412, 0, 1}}, // Green
		{{-0.25f, -0.25f, -0.25f, 1},{0.6, 0, 0.749, 1}}, // Purple

		//right
		{{0, 1.0f, 0, 1},{0.9608, 0.651, 0.1137, 1}}, // Orange
		{{0.25f, -0.25f, 0.25f, 1},{0.0627, 0.7412, 0, 1}}, // Green
		{{0.25f, -0.25f, -0.25f, 1},{0.6, 0, 0.749, 1}}, // Purple

		//back
		{{0, 1.0f, 0, 1},{0.9608, 0.651, 0.1137, 1}}, // Orange
		{{-0.25f, -0.25f, 0.25f, 1},{0.0627, 0.7412, 0, 1}}, // Green
		{{0.25f, -0.25f, 0.25f, 1},{0.6, 0, 0.749, 1}}, // Purple

		//left
		{{0, 1.0f, 0, 1},{0.9608, 0.651, 0.1137, 1}}, // Orange
		{{-0.25f, -0.25f, -0.25f, 1},{0.0627, 0.7412, 0, 1}}, // Green
		{{-0.25f, -0.25f, 0.25f, 1},{0.6, 0, 0.749, 1}}, // Purple

	};

	numVerts = ARRAYSIZE(triangle);

	// Load triangle on card
	D3D11_BUFFER_DESC bDesc;
	D3D11_SUBRESOURCE_DATA subData;
	ZeroMemory(&bDesc, sizeof(bDesc));
	ZeroMemory(&subData, sizeof(subData));

	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(Vertex) * numVerts;
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_DEFAULT;

	subData.pSysMem = triangle;

	dev->CreateBuffer(&bDesc, &subData, &vBuff);

	// write, compile and load the shaders
	dev->CreateVertexShader(VertexShader, sizeof(VertexShader), NULL, &vShader);
	dev->CreatePixelShader(PixelShader, sizeof(PixelShader), NULL, &pShader);
	dev->CreateVertexShader(MeshVertexShader, sizeof(MeshVertexShader), NULL, &vMeshShader);

	// Describe it to dx11
	D3D11_INPUT_ELEMENT_DESC iDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	dev->CreateInputLayout(iDesc, 2, VertexShader, sizeof(VertexShader), &vLayout);

	// Create constant buffer
	ZeroMemory(&bDesc, sizeof(bDesc));

	bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bDesc.ByteWidth = sizeof(WVP);
	bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_DYNAMIC;

	dev->CreateBuffer(&bDesc, NULL, &cBuff);

	// Load our complex mesh onto the card
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(Sword_data);
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_IMMUTABLE;

	subData.pSysMem = Sword_data;
	dev->CreateBuffer(&bDesc, &subData, &vBuffMesh);

	// Index buffer mesh
	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = sizeof(Sword_indicies);

	subData.pSysMem = Sword_indicies;
	dev->CreateBuffer(&bDesc, &subData, &iBuffMesh);

	// Make matching input layout for mesh vertex
	D3D11_INPUT_ELEMENT_DESC meshInputDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	dev->CreateInputLayout(meshInputDesc, 3, MeshVertexShader, sizeof(MeshVertexShader), &vMeshLayout);

	// Create Z buffer & view
	D3D11_TEXTURE2D_DESC zDesc;
	ZeroMemory(&zDesc, sizeof(zDesc));
	zDesc.ArraySize = 1;
	zDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL; 
	zDesc.Width = swapDesc.BufferDesc.Width;
	zDesc.Height = swapDesc.BufferDesc.Height;
	zDesc.Format = DXGI_FORMAT_D32_FLOAT;
	zDesc.Usage = D3D11_USAGE_DEFAULT;
	zDesc.MipLevels = 1;
	zDesc.SampleDesc.Count = 1;

	dev->CreateTexture2D(&zDesc, NULL, &zBuffer);

	dev->CreateDepthStencilView(zBuffer, NULL, &zBufferView);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	//case WM_PAINT:
	//{
	//	PAINTSTRUCT ps;
	//	HDC hdc = BeginPaint(hWnd, &ps);
	//	// TODO: Add any drawing code that uses hdc here...
	//	EndPaint(hWnd, &ps);
	//}
	//break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
