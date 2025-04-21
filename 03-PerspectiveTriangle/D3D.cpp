// Windows header files
#include <windows.h>
#include <stdio.h> // File I/O
#include <fstream>
#include <sstream>
#include <stdlib.h> // For Exit

// D3D11 Related header file
#include <d3d11.h>
#include <d3dcompiler.h>

#include "D3D.h"
#pragma warning(disable : 4838)
#include "XNAMath_204/xnamath.h"

// d3d Related
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace std;

// Macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

class D3D11App
{
private:
    HWND ghwnd;                                       // Handle to the window
    HDC ghdc;                                         // Handle to the device context
    HGLRC ghrc;                                       // Handle to the rendering context
    BOOL gbFullscreen;                                // Fullscreen toggle flag
    BOOL gbActive;                                    // Active window flag
    DWORD dwStyle;                                    // Window style
    WINDOWPLACEMENT wpPrev;                           // Previous window placement
    IDXGISwapChain *gpIDXGISwapChain;                 // Swap chain interface
    ID3D11Device *gpID3D11Device;                     // Device interface
    ID3D11RenderTargetView *gpID3D11RenderTargetView; // Render target view interface
    ID3D11VertexShader *gpID3D11VertexShader;         // Vertex shader interface
    ID3D11PixelShader *gpID3D11PixelShader;           // Pixel shader interface
    ID3D11InputLayout *gpID3D11InputLayout;           // Input layout interface
    ID3D11Buffer *gpID3D11Buffer_PositionBuffer;      // Vertex buffer interface
    ID3D11Buffer *gpID3D11Buffer_ColorBuffer;         // Color buffer interface
    ID3D11Buffer *gpID3D11Buffer_ConstantBuffer;      // Constant buffer interface
    float gClearColor[4];                             // Clear color array

    struct CBUFFER
    {
        XMMATRIX WorldViewProjectionMatrix;
    };

    XMMATRIX perspectiveProjectionMatrix; // Orthographic projection matrix

public:
    FILE *gpFile;                               // Log file pointer
    ID3D11DeviceContext *gpID3D11DeviceContext; // Device context interface
    char gszLogFileName[256];

public:
    D3D11App();
    ~D3D11App();
    void setActive(BOOL active);           // Set active state
    BOOL isActive() const;                 // Check if active
    int Initialize(HWND hwnd);             // Initialize the application
    HRESULT Resize(int width, int height); // Resize the window
    void Render();                         // Render function
    void Update();                         // Update function
    void ToggleFullscreen();               // Toggle fullscreen mode
    void Cleanup();                        // Cleanup function

private:
    string readShaderSource(const char *filePath); // Read shader source code from file
    HRESULT setupShaders();                        // Setup shaders
    HRESULT setupBuffers();                        // Setup vertex buffers
};

D3D11App app; // Global instance of D3D11App

// Window Procedure
// Handles window messages
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;
    switch (iMsg)
    {
    case WM_SETFOCUS:
        app.setActive(TRUE); // Set app as active when window gains focus
        break;
    case WM_KILLFOCUS:
        app.setActive(FALSE); // Set app as inactive when window loses focus
        break;
    case WM_SIZE:
        if (app.gpID3D11DeviceContext)
        {
            hr = app.Resize(LOWORD(lParam), HIWORD(lParam)); // Handle window resizing
            if (FAILED(hr))
            {
                app.gpFile = fopen(app.gszLogFileName, "a+");
                fprintf(app.gpFile, "Resize Failed\n");
                fclose(app.gpFile);
                return hr;
            }
        }
        break;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) // Exit on ESC key press
        {
            DestroyWindow(hwnd);
        }
        else if (wParam == 'F' || wParam == 'f') // Toggle fullscreen on 'F' key press
        {
            app.ToggleFullscreen();
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd); // Destroy window on close
        break;
    case WM_DESTROY:
        PostQuitMessage(0); // Post quit message
        break;
    default:
        return DefWindowProc(hwnd, iMsg, wParam, lParam); // Default message handling
    }
    return 0;
}

// Entry Point
// Main function for the application
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    // Define and register the window class
    WNDCLASSEX wndclass = {sizeof(WNDCLASSEX)};
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;
    wndclass.hbrBackground = NULL;
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.lpszClassName = TEXT("D3D11App");
    wndclass.lpszMenuName = NULL;
    wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

    RegisterClassEx(&wndclass);

    // Create the application window
    HWND hwnd = CreateWindow(TEXT("D3D11App"), TEXT("D3D11 Application"),
                             WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                             WIN_WIDTH, WIN_HEIGHT, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    // Initialize the OpenGL application
    if (app.Initialize(hwnd) != 0)
    {
        MessageBox(hwnd, TEXT("Initialization failed"), TEXT("Error"), MB_OK | MB_ICONERROR);
        return 0;
    }

    // Main message loop
    MSG msg;
    BOOL bDone = FALSE;
    while (!bDone)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                bDone = TRUE; // Exit loop on quit message
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if (app.isActive()) // Render and update only if the app is active
            {
                app.Render();
                app.Update();
            }
        }
    }

    return (int)msg.wParam;
}

// D3D11App Class Implementation
// Constructor
D3D11App::D3D11App() : ghwnd(NULL),
                       ghdc(NULL),
                       ghrc(NULL),
                       gbFullscreen(FALSE),
                       gbActive(FALSE),
                       dwStyle(0),
                       wpPrev({sizeof(WINDOWPLACEMENT)}),
                       gpIDXGISwapChain(NULL),
                       gpID3D11Device(NULL),
                       gpID3D11DeviceContext(NULL),
                       gpID3D11RenderTargetView(NULL),
                       gpID3D11VertexShader(NULL),
                       gpID3D11PixelShader(NULL),
                       gpID3D11InputLayout(NULL),
                       gpID3D11Buffer_PositionBuffer(NULL),
                       gpID3D11Buffer_ColorBuffer(NULL),
                       gpID3D11Buffer_ConstantBuffer(NULL),
                       gpFile(NULL)

{
    strcpy_s(gszLogFileName, "Log.txt");
    gpFile = fopen(gszLogFileName, "w");
    if (gpFile == NULL)
    {
        MessageBox(NULL, TEXT("Log File Cannot be Opened"),
                   TEXT("Error"), MB_OK | MB_ICONERROR);
        return;
    }
    else
    {
        fprintf(gpFile, "Log File Created Successfully\n");
        fclose(gpFile);
    }
}

// Destructor
D3D11App::~D3D11App()
{
    Cleanup(); // Cleanup resources
}

// Set active state
void D3D11App::setActive(BOOL active)
{
    gbActive = active; // Set the active state
}

// Check if the application is active
BOOL D3D11App::isActive() const
{
    return gbActive; // Return the active state
}

// Toggle fullscreen mode
void D3D11App::ToggleFullscreen()
{
    MONITORINFO mi = {sizeof(MONITORINFO)};
    if (!gbFullscreen)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
        if (dwStyle & WS_OVERLAPPEDWINDOW)
        {
            if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
            {
                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                             mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
                             SWP_NOZORDER | SWP_FRAMECHANGED);
            }
        }
        ShowCursor(FALSE);
        gbFullscreen = TRUE;
    }
    else
    {
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        ShowCursor(TRUE);
        gbFullscreen = FALSE;
    }
}

// Initialize D3D11 and create swap chain, device, and render target view
int D3D11App::Initialize(HWND hwnd)
{
    HRESULT hr = S_OK;

    ghwnd = hwnd;

    // Initialize swap chain description
    DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
    ZeroMemory((void *)&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
    dxgiSwapChainDesc.BufferDesc.Width = WIN_WIDTH;
    dxgiSwapChainDesc.BufferDesc.Height = WIN_HEIGHT;
    dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    dxgiSwapChainDesc.SampleDesc.Count = 1;
    dxgiSwapChainDesc.SampleDesc.Quality = 0;
    dxgiSwapChainDesc.BufferCount = 1;
    dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    dxgiSwapChainDesc.OutputWindow = ghwnd;
    dxgiSwapChainDesc.Windowed = TRUE;

    // Create swap chain, device, device context and render target view
    D3D_DRIVER_TYPE d3dDriverType;
    D3D_DRIVER_TYPE d3dDriverTypes[] = {D3D_DRIVER_TYPE_HARDWARE,
                                        D3D_DRIVER_TYPE_WARP,
                                        D3D_DRIVER_TYPE_SOFTWARE,
                                        D3D_DRIVER_TYPE_REFERENCE,
                                        D3D_DRIVER_TYPE_NULL,
                                        D3D_DRIVER_TYPE_UNKNOWN};
    D3D_FEATURE_LEVEL d3dFeatureLevel_required = D3D_FEATURE_LEVEL_11_0;
    D3D_FEATURE_LEVEL d3dFeatureLevel_acquired = D3D_FEATURE_LEVEL_10_0;
    UINT numDriverTypes;
    UINT numFeatureLevels = 1;

    numDriverTypes = sizeof(d3dDriverTypes) / sizeof(d3dDriverTypes[0]);

    for (UINT i = 0; i < numDriverTypes; i++)
    {
        d3dDriverType = d3dDriverTypes[i];
        hr = D3D11CreateDeviceAndSwapChain(NULL,
                                           d3dDriverType,
                                           NULL,
                                           0,
                                           &d3dFeatureLevel_required,
                                           numFeatureLevels,
                                           D3D11_SDK_VERSION,
                                           &dxgiSwapChainDesc,
                                           &gpIDXGISwapChain,
                                           &gpID3D11Device,
                                           &d3dFeatureLevel_acquired,
                                           &gpID3D11DeviceContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "D3D11CreateDeviceAndSwapChain Failed\n");
        fclose(gpFile);
        return hr;
    }
    else
    {
        // print which driver we found
        gpFile = fopen(gszLogFileName, "a+");
        if (d3dDriverType == D3D_DRIVER_TYPE_HARDWARE)
        {
            fprintf(gpFile, "D3D11CreateDeviceAndSwapChain Succeeded with Hardware Driver\n");
        }
        else if (d3dDriverType == D3D_DRIVER_TYPE_WARP)
        {
            fprintf(gpFile, "D3D11CreateDeviceAndSwapChain Succeeded with WARP\n");
        }
        else if (d3dDriverType == D3D_DRIVER_TYPE_SOFTWARE)
        {
            fprintf(gpFile, "D3D11CreateDeviceAndSwapChain Succeeded with SOFTWARE\n");
        }
        else if (d3dDriverType == D3D_DRIVER_TYPE_REFERENCE)
        {
            fprintf(gpFile, "D3D11CreateDeviceAndSwapChain Succeeded with REFERENCE\n");
        }
        else if (d3dDriverType == D3D_DRIVER_TYPE_NULL)
        {
            fprintf(gpFile, "D3D11CreateDeviceAndSwapChain Succeeded with NULL\n");
        }
        else if (d3dDriverType == D3D_DRIVER_TYPE_UNKNOWN)
        {
            fprintf(gpFile, "D3D11CreateDeviceAndSwapChain Succeeded with UNKNOWN\n");
        }
        else
        {
            fprintf(gpFile, "D3D11CreateDeviceAndSwapChain Succeeded with Undefined Driver\n");
        }

        // which feature level we found
        if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_11_0)
        {
            fprintf(gpFile, "D3D11CreateDeviceAndSwapChain Succeeded with Feature Level 11.0\n");
        }
        else if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_10_1)
        {
            fprintf(gpFile, "D3D11CreateDeviceAndSwapChain Succeeded with Feature Level 10.1\n");
        }
        else if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_10_0)
        {
            fprintf(gpFile, "D3D11CreateDeviceAndSwapChain Succeeded with Feature Level 10.0\n");
        }
        else
        {
            fprintf(gpFile, "D3D11CreateDeviceAndSwapChain Succeeded with Feature Level UNKNOWN\n");
        }

        fclose(gpFile);
    }

    // Set up shaders
    setupShaders();

    // Set up buffers
    setupBuffers();

    gClearColor[0] = 0.0f;
    gClearColor[1] = 0.0f;
    gClearColor[2] = 0.0f;
    gClearColor[3] = 1.0f;

    perspectiveProjectionMatrix = XMMatrixIdentity();

    // warmup resize
    hr = Resize(WIN_WIDTH, WIN_HEIGHT);
    if (FAILED(hr))
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "Resize Failed\n");
        fclose(gpFile);
        return hr;
    }
    else
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "Resized Successfully\n");
        fclose(gpFile);
    }

    return hr;
}

HRESULT D3D11App::setupShaders()
{
    HRESULT hr = S_OK;
    // Vertex Shader
    std::string vertexShaderSourceCode = readShaderSource("vertexShader.hlsl");

    ID3DBlob *pID3DBlob_VertexShaderSourceCode = NULL;
    ID3DBlob *pID3DBlob_Error = NULL;

    // Compile above Shader
    hr = D3DCompile(vertexShaderSourceCode.c_str(),
                    vertexShaderSourceCode.length(),
                    "VS",
                    NULL,
                    D3D_COMPILE_STANDARD_FILE_INCLUDE,
                    "main",
                    "vs_5_0",
                    0,
                    0,
                    &pID3DBlob_VertexShaderSourceCode,
                    &pID3DBlob_Error);

    if (FAILED(hr))
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "Vertex Shader Compiling Error = %s\n", (char *)pID3DBlob_Error->GetBufferPointer());
        fclose(gpFile);
        return hr;
    }
    else
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "Vertex Shader Compiling Successful\n");
        fclose(gpFile);
    }

    // create the vertex shader from above code
    hr = gpID3D11Device->CreateVertexShader(pID3DBlob_VertexShaderSourceCode->GetBufferPointer(),
                                            pID3DBlob_VertexShaderSourceCode->GetBufferSize(),
                                            NULL,
                                            &gpID3D11VertexShader);

    if (FAILED(hr))
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "CreateVertexShader Failed\n");
        fclose(gpFile);
        return hr;
    }
    else
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "CreateVertexShader Successful\n");
        fclose(gpFile);
    }

    // set above created vertex shader in pipeline
    gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShader, NULL, 0);

    // Pixel Shader
    string pixelShaderSourceCode = readShaderSource("pixelShader.hlsl");

    ID3DBlob *pID3DBlob_PixelShaderSourceCode = NULL;
    pID3DBlob_Error = NULL;

    // Compile above Shader
    hr = D3DCompile(pixelShaderSourceCode.c_str(),
                    pixelShaderSourceCode.length(),
                    "PS",
                    NULL,
                    D3D_COMPILE_STANDARD_FILE_INCLUDE,
                    "main",
                    "ps_5_0",
                    0,
                    0,
                    &pID3DBlob_PixelShaderSourceCode,
                    &pID3DBlob_Error);

    if (FAILED(hr))
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "Pixel Shader Compiling Error = %s\n", (char *)pID3DBlob_Error->GetBufferPointer());
        fclose(gpFile);
        return hr;
    }
    else
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "Pixel Shader Compiling Successful\n");
        fclose(gpFile);
    }

    // create the Pixel shader from above code
    hr = gpID3D11Device->CreatePixelShader(pID3DBlob_PixelShaderSourceCode->GetBufferPointer(),
                                           pID3DBlob_PixelShaderSourceCode->GetBufferSize(),
                                           NULL,
                                           &gpID3D11PixelShader);

    if (FAILED(hr))
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "CreatePixelShader Failed\n");
        fclose(gpFile);
        return hr;
    }
    else
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "CreatePixelShader Successful\n");
        fclose(gpFile);
    }

    // set above created Pixel shader in pipeline
    gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader, NULL, 0);

    // release Error Blob
    if (pID3DBlob_Error)
    {
        pID3DBlob_Error->Release();
        pID3DBlob_Error = NULL;
    }

    // release pixel blob
    if (pID3DBlob_PixelShaderSourceCode)
    {
        pID3DBlob_PixelShaderSourceCode->Release();
        pID3DBlob_PixelShaderSourceCode = NULL;
    }

    // initialise input element structure
    D3D11_INPUT_ELEMENT_DESC d3dInputElementDesc[2];
    ZeroMemory((void *)d3dInputElementDesc, sizeof(D3D11_INPUT_ELEMENT_DESC) * _ARRAYSIZE(d3dInputElementDesc));

    // Position
    d3dInputElementDesc[0].SemanticName = "POSITION";
    d3dInputElementDesc[0].SemanticIndex = 0;
    d3dInputElementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    d3dInputElementDesc[0].InputSlot = 0;
    d3dInputElementDesc[0].AlignedByteOffset = 0;
    d3dInputElementDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    d3dInputElementDesc[0].InstanceDataStepRate = 0;

    // Color
    d3dInputElementDesc[1].SemanticName = "COLOR";
    d3dInputElementDesc[1].SemanticIndex = 0;
    d3dInputElementDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    d3dInputElementDesc[1].InputSlot = 1;
    d3dInputElementDesc[1].AlignedByteOffset = 0;
    d3dInputElementDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    d3dInputElementDesc[1].InstanceDataStepRate = 0;

    // using above structure create input layout
    hr = gpID3D11Device->CreateInputLayout(d3dInputElementDesc,
                                           _ARRAYSIZE(d3dInputElementDesc),
                                           pID3DBlob_VertexShaderSourceCode->GetBufferPointer(),
                                           pID3DBlob_VertexShaderSourceCode->GetBufferSize(),
                                           &gpID3D11InputLayout);
    if (FAILED(hr))
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "CreateInputLayout Failed\n");
        fclose(gpFile);
        return hr;
    }
    else
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "CreateInputLayout Successful\n");
        fclose(gpFile);
    }

    // set above input layout into pipeline
    gpID3D11DeviceContext->IASetInputLayout(gpID3D11InputLayout);

    // now we can release vertex shader blob
    if (pID3DBlob_VertexShaderSourceCode)
    {
        pID3DBlob_VertexShaderSourceCode->Release();
        pID3DBlob_VertexShaderSourceCode = NULL;
    }

    return hr;
}

// Function to set up buffers
HRESULT D3D11App::setupBuffers()
{
    HRESULT hr = S_OK;

    // declare triangle geometry
    const float triangle_Position[] =
        {
            0.0f, 1.0f, 0.0f,
            1.0, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f};

    const float triangle_Color[] =
        {
            1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f};

    // create vertex buffer from above Data
    // position
    D3D11_BUFFER_DESC d3dBufferDesc;
    ZeroMemory((void *)&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
    d3dBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    d3dBufferDesc.ByteWidth = sizeof(triangle_Position);
    d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    d3dBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    // create vertex buffer using above structure
    hr = gpID3D11Device->CreateBuffer(&d3dBufferDesc, NULL, &gpID3D11Buffer_PositionBuffer);
    if (FAILED(hr))
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "CreateBuffer Failed for Vertex Buffer\n");
        fclose(gpFile);
        return hr;
    }
    else
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "CreateBuffer Successful for Vertex Buffer\n");
        fclose(gpFile);
    }

    // We will set above buffer into pipeline not here but in Display
    // Copy manually data from cpu buffer to gpu buffer
    D3D11_MAPPED_SUBRESOURCE d3dMappedSubresource;
    ZeroMemory((void *)&d3dMappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));

    // now map our position buffer with above resource
    gpID3D11DeviceContext->Map(gpID3D11Buffer_PositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedSubresource);

    // now copy actual data
    memcpy(d3dMappedSubresource.pData, triangle_Position, sizeof(triangle_Position));

    gpID3D11DeviceContext->Unmap(gpID3D11Buffer_PositionBuffer, 0);

    // Color
    ZeroMemory((void *)&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
    d3dBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    d3dBufferDesc.ByteWidth = sizeof(triangle_Color);
    d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    d3dBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    // create vertex buffer using above structure
    hr = gpID3D11Device->CreateBuffer(&d3dBufferDesc, NULL, &gpID3D11Buffer_ColorBuffer);
    if (FAILED(hr))
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "CreateBuffer Failed for Vertex Buffer\n");
        fclose(gpFile);
        return hr;
    }
    else
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "CreateBuffer Successful for Vertex Buffer\n");
        fclose(gpFile);
    }

    // We will set above buffer into pipeline not here but in Display
    // Copy manually data from cpu buffer to gpu buffer
    ZeroMemory((void *)&d3dMappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));

    // now map our position buffer with above resource
    gpID3D11DeviceContext->Map(gpID3D11Buffer_ColorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedSubresource);

    // now copy actual data
    memcpy(d3dMappedSubresource.pData, triangle_Color, sizeof(triangle_Color));

    gpID3D11DeviceContext->Unmap(gpID3D11Buffer_ColorBuffer, 0);

    // create constant buffer to send tranformation like uniform data
    ZeroMemory((void *)&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
    d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    d3dBufferDesc.ByteWidth = sizeof(CBUFFER);
    d3dBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    // create Constant buffer using above structure
    hr = gpID3D11Device->CreateBuffer(&d3dBufferDesc, NULL, &gpID3D11Buffer_ConstantBuffer);
    if (FAILED(hr))
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "CreateBuffer Failed for Constant Buffer\n");
        fclose(gpFile);
        return hr;
    }
    else
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "CreateBuffer Successful for Constant Buffer\n");
        fclose(gpFile);
    }

    // set above buffer into pipeline
    gpID3D11DeviceContext->VSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer);

    return hr;
}

// Resize the swap chain and render target view
HRESULT D3D11App::Resize(int width, int height)
{
    // variable dec
    HRESULT hr = S_OK;

    // release rtv
    if (gpID3D11RenderTargetView)
    {
        gpID3D11RenderTargetView->Release();
        gpID3D11RenderTargetView = NULL;
    }

    // resize the swapchain buffers according to the changese size

    hr = gpIDXGISwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    if (FAILED(hr))
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "ResizeBuffers Failed\n");
        fclose(gpFile);
        return hr;
    }

    // create new reender target view
    // A. get the buffer from rtv from swapchain into the texture (just like FBO)
    ID3D11Texture2D *pID3D11texture2d = NULL;
    hr = gpIDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&pID3D11texture2d);
    if (FAILED(hr))
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "GetBuffer Failed\n");
        fclose(gpFile);
        return hr;
    }

    // B. create new rtv using above buffer
    hr = gpID3D11Device->CreateRenderTargetView(pID3D11texture2d, NULL, &gpID3D11RenderTargetView);
    if (FAILED(hr))
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "CreateRenderTargetView Failed\n");
        fclose(gpFile);
        return hr;
    }
    else
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "CreateRenderTargetView Successful\n");
        fclose(gpFile);
    }

    pID3D11texture2d->Release();
    pID3D11texture2d = NULL;

    // C. set this new rtv into OM state pipeline
    gpID3D11DeviceContext->OMSetRenderTargets(1, &gpID3D11RenderTargetView, NULL);

    // set the viewport
    D3D11_VIEWPORT d3dViewport;
    ZeroMemory((void *)&d3dViewport, sizeof(D3D11_VIEWPORT));
    d3dViewport.TopLeftX = 0.0f;
    d3dViewport.TopLeftY = 0.0f;
    d3dViewport.Width = (float)width;
    d3dViewport.Height = (float)height;
    d3dViewport.MinDepth = 0.0f;
    d3dViewport.MaxDepth = 1.0f;

    // set above viewport in pipeline
    gpID3D11DeviceContext->RSSetViewports(1, &d3dViewport);

    // initialise perpective projection matrix
    perspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

    // Code
    return hr;
}

// Render the scene
void D3D11App::Render()
{
    // Code
    // clear the rtv using clear color
    gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, gClearColor);

    // transformations
    XMMATRIX worldMatrix = XMMatrixIdentity();
    worldMatrix = XMMatrixTranslation(0.0f, 0.0f, 3.0f);
    XMMATRIX viewMatrix = XMMatrixIdentity();
    XMMATRIX wvpMatrix = XMMatrixIdentity();

    wvpMatrix = worldMatrix * viewMatrix * perspectiveProjectionMatrix;

    CBUFFER constantBuffer;
    ZeroMemory((void *)&constantBuffer, sizeof(CBUFFER));

    constantBuffer.WorldViewProjectionMatrix = wvpMatrix;

    gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);

    // set position buffer into pipeline Here
    // Position
    UINT stride = sizeof(float) * 3;
    UINT offset = 0;

    gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_PositionBuffer, &stride, &offset);

    // Color
    stride = sizeof(float) * 3;
    offset = 0;

    gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11Buffer_ColorBuffer, &stride, &offset);

    // set primitive geometry
    gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // draw the geometry
    gpID3D11DeviceContext->Draw(3, 0);

    // do double buffering by presenting the swapchain
    gpIDXGISwapChain->Present(0, 0);
}

// Update the application state
void D3D11App::Update()
{
    // Update logic can be added here
}

// Cleanup resources
void D3D11App::Cleanup()
{
    if (gbFullscreen == TRUE)
    {
        ToggleFullscreen();
        gbFullscreen = FALSE;
    }

    // Destroy Window
    if (ghwnd)
    {
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }

    if (gpID3D11Buffer_ConstantBuffer)
    {
        gpID3D11Buffer_ConstantBuffer->Release();
        gpID3D11Buffer_ConstantBuffer = NULL;
    }

    if (gpID3D11Buffer_ColorBuffer)
    {
        gpID3D11Buffer_ColorBuffer->Release();
        gpID3D11Buffer_ColorBuffer = NULL;
    }

    if (gpID3D11Buffer_PositionBuffer)
    {
        gpID3D11Buffer_PositionBuffer->Release();
        gpID3D11Buffer_PositionBuffer = NULL;
    }

    if (gpID3D11InputLayout)
    {
        gpID3D11InputLayout->Release();
        gpID3D11InputLayout = NULL;
    }

    if (gpID3D11InputLayout)
    {
        gpID3D11InputLayout->Release();
        gpID3D11InputLayout = NULL;
    }

    if (gpID3D11PixelShader)
    {
        gpID3D11PixelShader->Release();
        gpID3D11PixelShader = NULL;
    }

    if (gpID3D11VertexShader)
    {
        gpID3D11VertexShader->Release();
        gpID3D11VertexShader = NULL;
    }

    if (gpID3D11DeviceContext)
    {
        gpID3D11DeviceContext->Release();
        gpID3D11DeviceContext = NULL;
    }

    if (gpIDXGISwapChain)
    {
        gpIDXGISwapChain->Release();
        gpIDXGISwapChain = NULL;
    }

    if (gpID3D11Device)
    {
        gpID3D11Device->Release();
        gpID3D11Device = NULL;
    }

    if (gpFile)
    {
        fprintf(gpFile, "Closing log file\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}

string D3D11App::readShaderSource(const char *filePath)
{
    std::ifstream shaderFile(filePath, std::ios::in);
    if (!shaderFile.is_open())
    {
        gpFile = fopen(gszLogFileName, "a+");
        fprintf(gpFile, "Failed to open shader file: %s\n", filePath);
        fclose(gpFile);
        return "";
    }

    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();

    return shaderStream.str();
}
