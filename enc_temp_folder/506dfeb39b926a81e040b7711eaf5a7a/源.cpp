
///////////////**************new**************////////////////////

//Include and link appropriate libraries and headers//
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment (lib, "DXErr.lib")
#pragma comment(lib, "legacy_stdio_definitions.lib")
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <xnamath.h>
#include <DxErr.h>
//Define variables/constants//
LPCTSTR WndClassName = "firstwindow";
HWND hwnd = NULL;	//Sets windows handle to NULL
HRESULT hr;
const int Width = 800;		//window width
const int Height = 600;		//window hegith

IDXGISwapChain* SwapChain;
ID3D11Device* d3d11Device;
ID3D11DeviceContext* d3d11DevCon;
ID3D11RenderTargetView* renderTargetView;
ID3D11Buffer * triangleVertBuffer;
ID3D11VertexShader* VS;
ID3D11PixelShader* PS;
ID3D10Blob* VS_Buffer;
ID3D10Blob* PS_Buffer;
ID3D11InputLayout* verLayout;

float red = 0.0f;
float green = 0.0f;
float blue = 0.0f;
int colormodr = 1;
int colormodg = 1;
int colormodb = 1;



//Functions//
bool InitializeWindow(HINSTANCE hInstance,		//Initialize window
	int ShowWnd,
	int width, int height,
	bool windowed);

int messageLoop();		//Main part of the program;

LRESULT CALLBACK WndProc(		//Windows callback procedure
	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
);

//directx11 functions
bool InitializeDirect3d11App(HINSTANCE hInstance);
void ReleaseObjects();
bool InitScene();
void UpdateScene();
void DrawScene();

//Vertex Structure and Vertex Layout (Input Layout)
struct Vertex  //Overload Vertex Structure
{
	Vertex(){}
	Vertex(float x , float y, float z):pos(x,y,z){}
	XMFLOAT3 pos;
};

D3D11_INPUT_ELEMENT_DESC layout[] =
{
	{"POSITION",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
};
UINT numElements = ARRAYSIZE(layout); // hold the size of input layout array

void CleanUp() {
	//Release the COM objects we created
	SwapChain->Release();
	d3d11Device->Release();
	d3d11DevCon->Release();
	renderTargetView->Release();
	triangleVertBuffer->Release();
	PS->Release();
	VS->Release();
	VS_Buffer->Release();
	PS_Buffer->Release();
	verLayout->Release();

}

//-0------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,		//Main windows function
	HINSTANCE hPrevinstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	//initialize Window//
	if (!InitializeWindow(hInstance, nShowCmd, Width, Height, true))
	{
		MessageBox(0, "Window Initialization - Failed",
			"Error", MB_OK);
		return 0;
	}
	if (!InitializeDirect3d11App(hInstance))
	{
		MessageBox(0, "Direct3D Initialization - Failed", "Error", MB_OK);
		return 0;
	}
	if (!InitScene())    //Initialize our scene
	{
		MessageBox(0, "Scene Initialization - Failed",
			"Error", MB_OK);
		return 0;
	}


	messageLoop();
	ReleaseObjects();
	return 0;
}


//init directx
bool InitializeDirect3d11App(HINSTANCE hInstance)
{

	HRESULT hr;

	//Describe Buffer
	DXGI_MODE_DESC buffDesc;
	//typedef struct DXGI_MODE_DESC {
	//	UINT                     Width;
	//	UINT                     Height;
	//	DXGI_RATIONAL            RefreshRate;
	//	DXGI_FORMAT              Format;
	//	DXGI_MODE_SCANLINE_ORDER ScanlineOrdering;
	//	DXGI_MODE_SCALING        Scaling;
	//} DXGI_MODE_DESC, *LPDXGI_MODE_DESC;

	ZeroMemory(&buffDesc, sizeof(DXGI_MODE_DESC)); // calls ZeroMemory to make sure the object
												   //is completely cleaned out
	buffDesc.Width = Width;
	buffDesc.Height = Height;
	buffDesc.RefreshRate.Denominator = 1;
	buffDesc.RefreshRate.Numerator = 60;
	buffDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	buffDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	buffDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	//Describe our SwapChain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferDesc = buffDesc;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//A DXGI_USAGE enumerated type describing the access the cpu has to the surface of the back buffer
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.SampleDesc.Count = 1;		//DXGI_SAMPLE_DESC structure describes the multisampling.
											//multisampling is used to "smooth" out the choppiness in lines and edges
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.Windowed = true;


	//create SwapChain
	hr = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL, NULL, NULL, NULL,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&SwapChain,
		&d3d11Device,
		NULL,
		&d3d11DevCon); //the device context will be used for the 
						//rendering methods of the device, to support multi-threading and boost performance
	if (FAILED(hr))
	{
		MessageBox(NULL, DXGetErrorDescription(hr),
			TEXT("D3D11CreateDeviceAndSwapChain"), MB_OK);
		return 0;
	}
	//create BackBuffer
	ID3D11Texture2D* BackBuffer;
	hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&BackBuffer);

	if (FAILED(hr))
	{
		MessageBox(NULL, DXGetErrorDescription(hr),
			TEXT("SwapChian"), MB_OK);
		return 0;
	}

	//create Render Target
	hr = d3d11Device->CreateRenderTargetView(BackBuffer, NULL, &renderTargetView);
	BackBuffer->Release();
	if (FAILED(hr))
	{
		MessageBox(NULL, DXGetErrorDescription(hr),
			TEXT("d3d11Device->CreateRenderTargetView"), MB_OK);
		return 0;
	}

	//Set Render Target
	d3d11DevCon->OMSetRenderTargets(1, &renderTargetView, NULL);
	return true;
}


void ReleaseObjects() {
	//Release the COM Objects we created;
	SwapChain->Release();
	d3d11DevCon->Release();
	d3d11Device->Release();
}

bool InitScene() {
	//Compile Shaders form shader file
	hr = D3DX11CompileFromFile("Effectx.fx", 0, 0, "VS", "vs_4_0", 0, 0, 0, &VS_Buffer, 0, 0);
	hr = D3DX11CompileFromFile("Effectx.fx", 0, 0, "PS", "ps_4_0", 0, 0, 0, &PS_Buffer,0,0);
	
	//Create the Shader Objects
	hr = d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(),
		NULL, &VS);
	hr = d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);



	//Set Vertex and Pixel Shaders
	d3d11DevCon->VSSetShader(VS, 0, 0);
	d3d11DevCon->PSSetShader(PS, 0, 0);
	
	//Create the vertex buffer
	Vertex v[] =
	{
		Vertex(0.0f,0.5f,0.5f),
		Vertex(0.0f, -0.5f, 0.5f),
		Vertex(-0.5f,-0.5f,0.5f)
	};

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc,sizeof(D3D11_BUFFER_DESC));
	
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * 3;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	vertexBufferData.pSysMem = v;
	hr = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &triangleVertBuffer);

	//set the vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	d3d11DevCon->IASetVertexBuffers(0, 1, &triangleVertBuffer, &stride, &offset);

	//Create the Input Layout
	hr = d3d11Device->CreateInputLayout(layout, numElements, VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), &verLayout);

	//Set the Input layout
	d3d11DevCon->IASetInputLayout(verLayout);

	//Set Primitive Topology
	d3d11DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Create the Viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Height = Height;
	viewport.Width = Width;
	

	//Set the ViewPort
	d3d11DevCon->RSSetViewports(1, &viewport);

	return true;



}

void UpdateScene() {
	
	//update the colors of our scene
	red += colormodr * 0.00005f;
	green += colormodg * 0.0005f;
	blue += colormodb * 0.002f;

	if (red >= 1 || red <= 0)
		colormodr *= -1;
	if (green >= 1 || green <= 0)
		colormodg *= -1;
	if (blue >= 1 || blue <= 0)
		colormodb *= -1;
	
		}

void DrawScene() {
	//Clear our backbuffer to the update color
	//D3DXCOLOR bgColor(red, green, blue, 1.0f);
	D3DXCOLOR bgColor(0.0f, 0.0f, 0.0f, 0.0f);
	d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);

	//Draw the triangle
	d3d11DevCon->Draw(3, 0);


	//Present the backbuffer to the screen;
	SwapChain->Present(0, 0);
}


bool InitializeWindow(
	HINSTANCE hInstance,
	int ShowWnd,
	int width, int height,
	bool windowed)
{
	//Start Creating the window

	WNDCLASSEX wc;		//Create a new extend window class

	wc.cbSize = sizeof(WNDCLASSEX);		//Size of our windows class
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;		//Extra bytes after out wc structure
	wc.cbWndExtra = NULL;		//Extra bytes after out windows instance
	wc.hInstance = hInstance;		//instance to current application
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);		//Title bar Icon
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);		//Default mouse Icon
	wc.lpszClassName = WndClassName;		//Name of our window class
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);		//Window bg color
	wc.lpszMenuName = NULL;		//Name of the menu attached to our window
	wc.hIconSm = LoadIcon(NULL, IDI_WINLOGO);	//Icon in your taskbar

	if (!RegisterClassEx(&wc))	//Register out window class
	{
		//if registration failed, dispaly error
		MessageBox(NULL, "Error registering class",
			"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	hwnd = CreateWindowEx(  // Create out Extended Window
		NULL,	//Extended Style
		WndClassName,	//Name of our window class
		"Window Title is This",	//Name in the title bar of our window
		WS_OVERLAPPEDWINDOW,	//style of out window
		CW_USEDEFAULT, CW_USEDEFAULT,	//Top left corner of window
		width,
		height,
		NULL,	//Handle to parent window
		NULL,	//Hanle to a Menu
		hInstance,	//Specifies instance of current program
		NULL	//used for an MDI client window
	);

	if (!hwnd)	//Make sure our has been created
	{
		//if not, dispaly error
		MessageBox(NULL, "Error creating window",
			"Error", MB_OK | MB_ICONERROR);
	}

	ShowWindow(hwnd, ShowWnd);	//Show our window
	UpdateWindow(hwnd);

	return true;
}

int messageLoop() {
	MSG msg;	//Create a new message structure
	ZeroMemory(&msg, sizeof(MSG));	//clear message structure to NULL

	while (true)		//while there is a message
	{
		//if there was a windows message
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) //if the message was WM_QUIT
				break;
			TranslateMessage(&msg);		//Translate the msg

			//send the message to default windows procedure
			DispatchMessage(&msg);
		}
		else {
			// Otherwise keep the flow going

			//*********************************
			//run game code
			UpdateScene();
			DrawScene();
		}
	}

	return msg.wParam;

}

LRESULT CALLBACK WndProc(	//Default windows procedure
	HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (msg)	//Check message
	{
	case WM_KEYDOWN:	//For a key down
		if (wParam == VK_ESCAPE) {
			if (MessageBox(0, "Are you sure you want to exit?"
				, "Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)

				//Release the windows allocated memeory
				DestroyWindow(hwnd);
		}
		return 0;
	case WM_DESTROY:	//if X button in top right was pressed
		PostQuitMessage(0);
		return 0;
	}
	//return the message for windows to handle it
	return DefWindowProc(
		hwnd,
		msg,
		wParam, lParam);
}

