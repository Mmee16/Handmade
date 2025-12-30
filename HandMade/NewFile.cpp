#include <windows.h>

#define internal static
#define global_var static



global_var bool running;
global_var void* bitmapMemory;
global_var BITMAPINFO bitmapinfo;
global_var HBITMAP bitmapHandle;
global_var HDC bitmapDeviceContext;

//DIB - Device independent Bitmap
// This function Creates a new bitmap when ever window size changes
/** While creating a new bitmap
Either you can first free the old buffer and create a new one or
You can create a new buffer then free the old one if this fails we can
fall back to first one
**/
internal void
ResizeDIBSection(int width, int height)
{
	if(bitmapHandle)
	{
		DeleteObject(bitmapHandle);
	}
	if(!bitmapDeviceContext)
	{
		bitmapDeviceContext = CreateCompatibleDC(0);
	}
	
	
	bitmapinfo.bmiHeader.biSize = sizeof(bitmapinfo.bmiHeader);
	bitmapinfo.bmiHeader.biWidth = width;
	bitmapinfo.bmiHeader.biHeight = height;
	bitmapinfo.bmiHeader.biPlanes = 1;
	bitmapinfo.bmiHeader.biBitCount = 32;
	bitmapinfo.bmiHeader.biCompression = BI_RGB;
	
	bitmapHandle = CreateDIBSection(
			0 , &bitmapinfo, DIB_RGB_COLORS,
			&bitmapMemory, 0,0);
	
}

internal void 
updateWindow(HDC deviceContext, int x, int y, int width, int height)
{
	StretchDIBits(
		deviceContext,
		x, y, width, height,
		x, y, width, height,
		bitmapMemory,
		&bitmapinfo,
		DIB_RGB_COLORS, SRCCOPY);
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
			GetClientRect(windowHandle, &clientRect);
			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;
			// When window size changes we want to redraw our things based on the new window size
			ResizeDIBSection(width, height);
			OutputDebugStringA("WM_SIZE\n");
			break;
		}
		// This message is posted when window is closed (X button, alt+f4,...)
		case WM_CLOSE:
		{
			running = false;
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
			PAINTSTRUCT paint;
			HDC deviceContext = BeginPaint(windowHandle, &paint);
			RECT rect = paint.rcPaint;
			int x = rect.left;
			int y = rect.top;
			int width = rect.right - rect.left;
			int height = rect.bottom - rect.top;
			static DWORD color = BLACKNESS;
			updateWindow(deviceContext, x, y, width, height);
			EndPaint(windowHandle, &paint);

			break;
		}
		default:
		{
			OutputDebugStringA("Default\n");
			return DefWindowProc( windowHandle, message, wParam, lParam);
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
	WNDCLASSA windowClass = {};
	windowClass.style = CS_OWNDC|CS_HREDRAW|CS_HREDRAW ; 
	windowClass.lpfnWndProc = MainCallBack; 
	windowClass.hInstance = hInstance /*GetModuleHandle(0)*/; 
	//windowClass.hIcon;  
	windowClass.lpszClassName = "HandMadeHero";
	
	if(RegisterClassA(&windowClass))
	{
		HWND windowHandle = 
			CreateWindowExA(
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
			running = true;
			MSG message;
			while(running)
			{
				//GetMessageA can give you -1 as well if error occurs so exit
				// when msgResult <= 0
				BOOL msgResult = GetMessageA(&message, 0, 0, 0);
				if(msgResult > 0)
				{
					TranslateMessage(&message);
					DispatchMessageA(&message);
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
