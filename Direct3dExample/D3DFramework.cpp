#include "D3DFramework.h"

void D3DFramework::InitWindow(HINSTANCE hInstance)
{
    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    mInstance = hInstance;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpszClassName = mClassName.c_str();
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpfnWndProc = WindowProc;
    wc.cbSize = sizeof(WNDCLASSEX);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(nullptr, L"Failed to register window class!", L"Error",
            MB_ICONEXCLAMATION | MB_OK);
        return ;
    }

    RECT wr{ 0, 0, mScreenWidth, mScreenHeight };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    mHwnd = CreateWindowEx(NULL,
        mClassName.c_str(),
        mTitle.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        wr.right - wr.left,
        wr.bottom - wr.top,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (mHwnd == nullptr)
    {
        MessageBox(nullptr, L"Failed to create window class!", L"Error",
            MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    SetWindowLongPtr(mHwnd, GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(this));

    ShowWindow(mHwnd, SW_SHOW);
    SetForegroundWindow(mHwnd);
    SetFocus(mHwnd);
    UpdateWindow(mHwnd);
}

void D3DFramework::InitD3D()
{
    DXGI_SWAP_CHAIN_DESC scd;

    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
    scd.BufferCount = 1;                                    // BackBuffer의 갯수
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // PixelFormat
    scd.BufferDesc.Width = mScreenWidth;
    scd.BufferDesc.Height = mScreenHeight;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // BackBuffer는 RenderTarget으로 지정할 목적
    scd.OutputWindow = mHwnd;                               // 그림을 그릴 윈도우
    scd.SampleDesc.Count = 1;                               // 1xMSAA - Anti-Aliasing 안함
    scd.Windowed = TRUE;                                    // 창모드
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;     // 창모드 <-> 전체화면 전환


    D3D11CreateDeviceAndSwapChain(
        NULL,                                       // Adapter pointer
        D3D_DRIVER_TYPE_HARDWARE,                   // 물리적인 하드웨어(그래픽카드)
        NULL,                                       // 소프트웨어 렌더러
        NULL,                                       // 옵션
        NULL,                                       // D3D_FEATURE_LEVEL 배열 - 최소사양, const * >> 배열을 바꾸지 말라는 뜻
        0,                                          // 배열 크기
        D3D11_SDK_VERSION,                          // SDK
        &scd,                                       // DXGI_SWAP_CHAIN_DESC 구조체에 대한 *(포인터)
        mspSwapChain.ReleaseAndGetAddressOf(),      // 생성된 SwapChain 인터페이스의 포인터가 반환
        mspDevice.ReleaseAndGetAddressOf(),         // 생성된 Device 인터페이스
        NULL,                                       // 그래픽카드가 지원하는 DX 버전 목록
        mspDeviceContext.ReleaseAndGetAddressOf()   // 생성된 DeviceContext 인터페이스
    );

    OnResize();
}

void D3DFramework::OnResize()
{
    // SwapChain 크기 변경
    ID3D11RenderTargetView* nullView[] = { nullptr };
    mspDeviceContext->OMSetRenderTargets(_countof(nullView), nullView, nullptr);

    mspRenderTargetView.Reset();
    mspDepthStencilView.Reset();
    mspRenderTarget.Reset();
    mspDepthStencil.Reset();

    mspDeviceContext->Flush();
    mspSwapChain->ResizeBuffers(
        0,
        mScreenWidth,
        mScreenHeight,
        DXGI_FORMAT_UNKNOWN,
        0
    );



    // 스왑체인(Front, Back) <- Back 그릴예정 <- RenderTarget을 BackBuffer로 지정
    mspSwapChain->GetBuffer(0, IID_PPV_ARGS(mspRenderTarget.ReleaseAndGetAddressOf()));

    // resource -> resource view 연결
    mspDevice->CreateRenderTargetView(
        mspRenderTarget.Get(),
        nullptr,
        mspRenderTargetView.GetAddressOf()
    );

    // Depth-Stencil Buffer
    CD3D11_TEXTURE2D_DESC t2d(
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        mScreenWidth,
        mScreenHeight,
        1,
        1,
        D3D11_BIND_DEPTH_STENCIL
    );

    mspDevice->CreateTexture2D(&t2d, nullptr, mspDepthStencil.ReleaseAndGetAddressOf());

    CD3D11_DEPTH_STENCIL_VIEW_DESC dsvd(D3D11_DSV_DIMENSION_TEXTURE2D);

    mspDevice->CreateDepthStencilView(
        mspDepthStencil.Get(),
        &dsvd,
        mspDepthStencilView.ReleaseAndGetAddressOf()
    );

    // Output Merger 연결
    mspDeviceContext->OMSetRenderTargets(
        1,
        mspRenderTargetView.GetAddressOf(),
        mspDepthStencilView.Get()
    );

    // Viewport
    CD3D11_VIEWPORT viewport(
        0.0f, 0.0f,
        static_cast<float>(mScreenWidth),
        static_cast<float>(mScreenHeight)
    );
    mspDeviceContext->RSSetViewports(1, &viewport);

}

void D3DFramework::RenderFrame()
{
    float clear_color[4]{ 0.0f,0.2f,0.4f,1.0f };    // rgba UNORM

    mspDeviceContext->ClearRenderTargetView(mspRenderTargetView.Get(), clear_color);
    mspDeviceContext->ClearDepthStencilView(
        mspDepthStencilView.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        1.0f,
        0
    );

    Render();

    mspSwapChain->Present(0, 0);
}

void D3DFramework::Initialize(HINSTANCE hInstance, int width, int height)
{
	mScreenWidth = width;
	mScreenHeight = height;

	InitWindow(hInstance);
	InitD3D();
}

void D3DFramework::Destroy()
{
    mspSwapChain->SetFullscreenState(FALSE, nullptr);

    mspDepthStencilView.Reset();
    mspDepthStencil.Reset();
    mspRenderTargetView.Reset();
    mspRenderTarget.Reset();
    mspSwapChain.Reset();
    mspDeviceContext.Reset();
    mspDevice.Reset();

    DestroyWindow(mHwnd);
    UnregisterClass(mClassName.c_str(), mInstance);
}

void D3DFramework::GameLoop()
{
    MSG msg{};
    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                break;
            }
        }
        else
        {
            // Game Loop
            RenderFrame();
        }
    }
}

LRESULT D3DFramework::MessageHandle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    {
        switch (message)
        {
        case WM_PAINT:
            if (mResizing)
            {
                RenderFrame();
            }
            else
            {
                PAINTSTRUCT ps;
                BeginPaint(hWnd, &ps);
                EndPaint(hWnd, &ps);
            }
            break;

        case WM_ENTERSIZEMOVE:
            mResizing = true;
            return 0;
            break;

        case WM_SIZE:
            mScreenWidth = LOWORD(lParam);
            mScreenHeight = HIWORD(lParam);
            if (mspDevice)
            {
                if (wParam == SIZE_MINIMIZED)
                {
                    mMaxmized = true;
                    mMaxmized = false;
                }
                else if (wParam == SIZE_MAXIMIZED)
                {
                    mMinmized = false;
                    mMaxmized = true;
                    OnResize();
                }
                else if (wParam == SIZE_RESTORED)
                {
                    if (mMinmized)
                    {
                        mMinmized = false;
                        OnResize();
                    }
                    else if (mMaxmized)
                    {
                        mMaxmized = false;
                        OnResize();
                    }
                    else if (mResizing)
                    {

                    }
                    else
                    {
                        OnResize();
                    }
                }
                else
                {
                    // 사용자가 드래그 중
                }
            }

            return 0;
            break;

        case WM_GETMINMAXINFO:
            reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.x = 640;
            reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.x = 480;
            break;

        case WM_MENUCHAR:
            return MAKELRESULT(0, MNC_CLOSE);
            break;

        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }

        return 0;
    }
}

LRESULT WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    D3DFramework* pFramework = reinterpret_cast<D3DFramework*>(
        GetWindowLongPtr(hWnd, GWLP_USERDATA)
        );

    return pFramework->MessageHandle(hWnd, message, wParam, lParam);
}
