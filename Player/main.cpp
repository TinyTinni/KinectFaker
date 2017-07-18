#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include <KinectFileDef.pb.h> //GOOGLE_PROTOBUF_VERIFY_VERSION

#include "render.h"

#pragma comment (lib, "d3d11")
#pragma comment (lib, "d3dcompiler")

#include <QSurfaceFormat>
#include "main_window.h"

int main(int argc, char *argv[])
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    QSurfaceFormat glContextOptions;
#ifdef _DEBUG
    glContextOptions.setOption(QSurfaceFormat::DebugContext);
#endif
    glContextOptions.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(glContextOptions);

    QApplication app{ argc,argv };
    main_window m;
    m.show();
    return app.exec();
}

//D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
//D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
//ID3D11Device*           g_pd3dDevice = nullptr;
//ID3D11DeviceContext*    g_pImmediateContext = nullptr;
//IDXGISwapChain*         g_pSwapChain = nullptr;
//ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
//const auto safe_rel = [](auto p) {if (p)p->Release(); };
//
//void CleanupDevices()
//{
//    safe_rel(g_pRenderTargetView);
//    safe_rel(g_pSwapChain);
//    safe_rel(g_pImmediateContext);
//    safe_rel(g_pd3dDevice);
//
//}
//
////--------------------------------------------------------------------------------------
//// Called every time the application receives a message
////--------------------------------------------------------------------------------------
//LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//    PAINTSTRUCT ps;
//    HDC hdc;
//  
//    switch (message)
//    {
//    case WM_PAINT:
//        hdc = BeginPaint(hWnd, &ps);
//        EndPaint(hWnd, &ps);
//        break;
//
//    case WM_DESTROY:
//        PostQuitMessage(0);
//        break;
//
//        // Note that this tutorial does not handle resizing (WM_SIZE) requests,
//        // so we created the window without the resize border.
//
//    default:
//        return DefWindowProc(hWnd, message, wParam, lParam);
//    }
//
//    return 0;
//}
//
//HRESULT InitDevice(HWND hwnd, const unsigned width, const unsigned height)
//{
//    HRESULT hr = S_OK;
//
//    UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
//#ifdef _DEBUG
//    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
//#endif
//
//    D3D_DRIVER_TYPE driverTypes[] =
//    {
//        D3D_DRIVER_TYPE_HARDWARE,
//        D3D_DRIVER_TYPE_WARP,
//        D3D_DRIVER_TYPE_REFERENCE,
//    };
//    UINT numDriverTypes = ARRAYSIZE(driverTypes);
//
//    D3D_FEATURE_LEVEL featureLevels[] =
//    {
//        D3D_FEATURE_LEVEL_11_0
//        /*D3D_FEATURE_LEVEL_11_1,
//        D3D_FEATURE_LEVEL_11_0,
//        D3D_FEATURE_LEVEL_10_1,
//        D3D_FEATURE_LEVEL_10_0,*/
//    };
//    UINT numFeatureLevels = ARRAYSIZE(featureLevels);
//
//    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
//    {
//        g_driverType = driverTypes[driverTypeIndex];
//        hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
//            D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
//
//        if (hr == E_INVALIDARG)
//        {
//            // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
//            hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
//                D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
//        }
//
//        if (SUCCEEDED(hr))
//            break;
//    }
//    if (FAILED(hr))
//        return hr;
//
//    // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
//    IDXGIFactory1* dxgiFactory = nullptr;
//    {
//        IDXGIDevice* dxgiDevice = nullptr;
//        hr = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
//        if (SUCCEEDED(hr))
//        {
//            IDXGIAdapter* adapter = nullptr;
//            hr = dxgiDevice->GetAdapter(&adapter);
//            if (SUCCEEDED(hr))
//            {
//                hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
//                adapter->Release();
//            }
//            dxgiDevice->Release();
//        }
//    }
//    if (FAILED(hr))
//        return hr;
//
//    // Create swap chain
//    {
//        // DirectX 11.0 systems
//        DXGI_SWAP_CHAIN_DESC sd;
//        ZeroMemory(&sd, sizeof(sd));
//        sd.BufferCount = 1;
//        sd.BufferDesc.Width = width;
//        sd.BufferDesc.Height = height;
//        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//        sd.BufferDesc.RefreshRate.Numerator = 60;
//        sd.BufferDesc.RefreshRate.Denominator = 1;
//        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//        sd.OutputWindow = hwnd;
//        sd.SampleDesc.Count = 1;
//        sd.SampleDesc.Quality = 0;
//        sd.Windowed = TRUE;
//
//        hr = dxgiFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);
//    }
//
//    // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
//    dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
//
//    dxgiFactory->Release();
//
//    if (FAILED(hr))
//        return hr;
//
//    // Create a render target view
//    ID3D11Texture2D* pBackBuffer = nullptr;
//    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
//    if (FAILED(hr))
//        return hr;
//
//    hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
//    pBackBuffer->Release();
//    if (FAILED(hr))
//        return hr;
//
//    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);
//
//    // Setup the viewport
//    D3D11_VIEWPORT vp;
//    vp.Width = (FLOAT)width;
//    vp.Height = (FLOAT)height;
//    vp.MinDepth = 0.0f;
//    vp.MaxDepth = 1.0f;
//    vp.TopLeftX = 0;
//    vp.TopLeftY = 0;
//    g_pImmediateContext->RSSetViewports(1, &vp);
//
//    return S_OK;
//}
//
//void Render()
//{
//    const FLOAT c[4] = { 0.f,0.f,0.f,1.f };
//    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, c);
//
//    
//    if (!Render(g_pd3dDevice, g_pImmediateContext))
//        return;
//
//
//    //ImGui::ShowTestWindow();
//    g_pSwapChain->Present(0, 0);
//}
//
//
//int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
//{
//    // Register class
//    WNDCLASSEX wcex;
//    wcex.cbSize = sizeof(WNDCLASSEX);
//    wcex.style = CS_HREDRAW | CS_VREDRAW;
//    wcex.lpfnWndProc = WndProc;
//    wcex.cbClsExtra = 0;
//    wcex.cbWndExtra = 0;
//    wcex.hInstance = hInstance;
//    wcex.hIcon = nullptr;
//    wcex.hCursor = nullptr;
//    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
//    wcex.lpszMenuName = nullptr;
//    wcex.lpszClassName = L"Player";
//    wcex.hIconSm = nullptr;
//    if (!RegisterClassEx(&wcex))
//        return E_FAIL;
//
//    // Create window
//    const unsigned width = 800;
//    const unsigned height = 600;
//    RECT rc = { 0,0,width, height };
//    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
//    HWND hWnd = CreateWindow(L"Player", L"Skeleton File Player",
//        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
//        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
//        nullptr);
//    if (!hWnd)
//        return -1;
//
//    ShowWindow(hWnd, nCmdShow);
//
//
//    if (FAILED(InitDevice(hWnd, width, height)))
//    {
//        CleanupDevices();
//        return 0;
//    }
//    Alloc_Ressources(g_pd3dDevice, g_pImmediateContext);
//
//    // Main message loop
//    MSG msg = { 0 };
//    while (WM_QUIT != msg.message)
//    {
//        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
//        {
//            TranslateMessage(&msg);
//            DispatchMessage(&msg);
//        }
//        else
//        {
//            Render();
//        }
//    }
//
//    Finish();
//    CleanupDevices();
//
//    return (int)msg.wParam;
//}