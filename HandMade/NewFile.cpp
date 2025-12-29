#include <windows.h>

#define internal static


//DIB - Device independent Bitmap
// This function Creates a new bitmap when ever window size changes
internal void
ResizeDIBSection(int width, int height)
{
	
}

LRESULT MainCallBack(
  HWND windowHandle,
  UINT message,
  WPARAM wParam,
  LPARAM lParam
)
{
	LRESULT result = 0;
	switch(message)
	{
		// This msg is sent when window size changes
		case WM_SIZE:
		{
			RECT clientRect;
			// This gives the actual part where you can draw into - 
			// Where as GetWindowRect gives total rect including scrollbar,buttons,...
			BOOL GetClientRect(hWnd, &clientRect);
			int width = clientRect.right - clientRect.left;
			int height = clientRect.top - clientRect.bottom;
			// When window size changes we want to redraw our things based on the new window size
			ResizeDIBSection(width, height);
			OutputDebugStringA("WM_SIZE\n");
			break;
		}
		// This message is posted when window is closed (X button, alt+f4,...)
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			OutputDebugStringA("WM_CLOSE\n");
			break;
		}
		case WM_DESTROY:
		{
			// Something destroyed the window handle the recreation of the window
			OutputDebugStringA("Something destroyed the window recreate it maybe\n");
			break;
		}
		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(windowHandle, &Paint);
			RECT rect = Paint.rcPaint;
			static DWORD color = BLACKNESS;
			PatBlt(DeviceContext, rect.left, rect.top, rect.right-rect.left, rect.top-rect.bottom, color);
			if(color == BLACKNESS)
			{
				color = WHITENESS;
			}
			else 
			{
				color = BLACKNESS;
			}
			EndPaint(windowHandle, &Paint);

			break;
		}
		default:
		{
			OutputDebugStringA("Default\n");
			result = DefWindowProc( windowHandle, message, wParam, lParam);
			break;
		}
	}
	return result;
}

int CALLBACK
WinMain(
  HINSTANCE hInstance, // Can be fetched from GetModuleHandle() Function,
  HINSTANCE hPrevInstance,
  LPSTR     lpCmdLine,
  int       nShowCmd)
{
	WNDCLASS windowClass = {};
	windowClass.style = CS_OWNDC|CS_HREDRAW|CS_HREDRAW ; 
	windowClass.lpfnWndProc = MainCallBack; 
	windowClass.hInstance = hInstance /*GetModuleHandle(0)*/; 
	//windowClass.hIcon;  
	windowClass.lpszClassName = "HandMadeHero";
	
	if(RegisterClass(&windowClass))
	{
		HWND windowHandle = 
			CreateWindowEx(
				0,
				windowClass.lpszClassName,
				"HandMade",
				WS_OVERLAPPEDWINDOW|WS_VISIBLE,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				0,
				0,
				hInstance,
				0);
		if(windowHandle)
		{
			MSG message;
			while(true)
			{
				BOOL msgResult = GetMessage(&message, 0, 0, 0);
				if(msgResult > 0)
				{
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
				else 
				{
					// Log error
					break;
				}
				
			}
		}
		else 
		{
			// Log error
		}
	}
	else 
	{
		// Logg as error
	}
	
	return 0;
}
