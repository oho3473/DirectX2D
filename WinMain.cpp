#include<Windows.h>
#include<d2d1.h>
#include<math.h>
#pragma comment(lib,"d2d1.lib")

const wchar_t gClassName[] = L"MyWindowClass";

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void OnPaint(HWND hwnd);

ID2D1Factory* gpD2DFactory{};
ID2D1HwndRenderTarget* gpRenderTarget{};

ID2D1SolidColorBrush* gpBrush{};
ID2D1RadialGradientBrush* gpRadialBrush{};

int WINAPI WinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{
	RECT wr{ 0,0,1024,768 };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hWnd;
	WNDCLASSEX wc;

	//팩토리 생성
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &gpD2DFactory);
	if (FAILED(hr))
	{
		MessageBox(nullptr, L"Failed to create D2D Factory", L"Error",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	//클래스 등록
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = gClassName;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpfnWndProc = WindowProc;
	wc.cbSize = sizeof(WNDCLASSEX);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(nullptr, L"Falied to register window class", L"Error",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	//윈도 생성
	hWnd = CreateWindowEx(NULL,
		gClassName,
		L"Direct2D",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wr.right-wr.left,
		wr.bottom-wr.top,
		NULL,
		NULL,
		hInstance,
		NULL);

	//렌더타켓 생성
	GetClientRect(hWnd, &wr);
	hr = gpD2DFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(
			hWnd,
			D2D1::SizeU(wr.right - wr.left, wr.bottom - wr.top)),
		&gpRenderTarget);
	if (FAILED(hr))
	{
		MessageBox(nullptr, L"Failed to create D2D RenderTarget", L"Error",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	hr = gpRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF(0x9ACD32, 1.0f)), &gpBrush);

	ID2D1GradientStopCollection* pGradientStops{};
	D2D1_GRADIENT_STOP gradientStops[2];

	gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::Yellow, 1);
	gradientStops[0].position = 0.0f;
	gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::Crimson, 1);
	gradientStops[1].position = 1.0f;

	hr = gpRenderTarget->CreateGradientStopCollection(gradientStops, 2,
		D2D1_GAMMA_2_2,
		D2D1_EXTEND_MODE_CLAMP,
		&pGradientStops);

	if (SUCCEEDED(hr))
	{
		hr = gpRenderTarget->CreateRadialGradientBrush(
			D2D1::RadialGradientBrushProperties(
				D2D1::Point2F(50, 150),
				D2D1::Point2F(0, 0),
				50,
				50),
			pGradientStops,
			&gpRadialBrush
		);
	}
	if (pGradientStops != nullptr)
	{
		pGradientStops->Release();
		pGradientStops = nullptr;
	}

	if (hWnd == nullptr)
	{
		MessageBox(nullptr, L"Failed to create window class", L"Error",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	//메시지 처리
	MSG msg{};
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if(msg.message == WM_QUIT)
			{
				break;
			}
		}
		else
		{
			OnPaint(hWnd);
		}
	}

	if (gpRenderTarget != nullptr)
	{
		gpRenderTarget->Release();
		gpRenderTarget = nullptr;
	}
	if (gpD2DFactory != nullptr)
	{
		gpD2DFactory->Release();
		gpD2DFactory = nullptr;
	}
	if (gpRadialBrush != nullptr)
	{
		gpRadialBrush->Release();
		gpRadialBrush = nullptr;
	}
	if (gpBrush != nullptr)
	{
		gpBrush->Release();
		gpBrush = nullptr;
	}

	return (int)msg.wParam;
}

void OnPaint(HWND hwnd)
{
	HDC hdc;
	PAINTSTRUCT ps;

	hdc = BeginPaint(hwnd, &ps);

	gpRenderTarget->BeginDraw();
	gpRenderTarget->Clear(D2D1::ColorF(0.0f, 0.2f, 0.4f, 1.0f));

	//불투명사각
	gpBrush->SetOpacity(1.0f);	//투명도
	gpBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Aquamarine));	//색상
	gpRenderTarget->FillRectangle(
		D2D1::RectF(0.0f, 0.0f, 100.0f, 100.0f),
		gpBrush
	);
	
	//반투명 사각
	gpBrush->SetOpacity(0.5f);	//투명도
	gpBrush->SetColor(D2D1::ColorF(D2D1::ColorF::LightYellow));	//색상
	gpRenderTarget->FillRectangle(
		D2D1::RectF(50.0f, 50.0f, 150.0f, 150.0f),
		gpBrush
	);
	
	static float fAngle = 0.0f;

	//타원
	gpBrush->SetOpacity(1.0f);	//투명도
	gpBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Red));	//색상
	gpRenderTarget->FillEllipse(
		D2D1::Ellipse(D2D1::Point2F(75.f+sinf(fAngle)*25, 150.0f), 50.0f, 70.0f),
			gpRadialBrush
	);

	fAngle += 0.2f;

	gpRenderTarget->EndDraw();

	EndPaint(hwnd, &ps);
}



LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_PAINT:
			OnPaint(hwnd);
			break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}