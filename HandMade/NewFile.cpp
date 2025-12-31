#include <windows.h>
#include <stdint.h>


#define internal static
#define global_var static


global_var bool running;

global_var void* bitmapMemory;
global_var BITMAPINFO bitmapinfo;

global_var int bitmapWidth;
global_var int bitmapHeight;
// bitmapMemory stores the info reg the color of each pixel
// So the size of bitmapMemory = #(pixels) * 4 (- 4 = 3 rgb colors 3 bytes + 1 byte for padding)
global_var int bytesPerPixel = 4;


internal void renderBitmap(int xOffSet, int yOffSet)
{
	int width = bitmapWidth;
	int height = bitmapHeight;
	
	uint8_t *row = (uint8_t *)bitmapMemory;
	int pitch = width * bytesPerPixel;
	for(int y=0; y<height; y++)
	{
		uint32_t *pixel = (uint32_t *)row;
		for(int x=0; x<width; x++)
		{
			uint8_t blue = (x+xOffSet);
			uint8_t green = (y+yOffSet);
			// Memory 	BB GG RR xx
			// Register xx RR GG BB - due to little endian
			*pixel++ = ((green << 8) | blue);
		}
		row+=pitch;
	}
}

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
	if(bitmapMemory)
	{
		VirtualFree(bitmapMemory, 0, MEM_RELEASE);
	}
	
	bitmapWidth = width;
	bitmapHeight = height;
	
	bitmapinfo.bmiHeader.biSize = sizeof(bitmapinfo.bmiHeader);
	bitmapinfo.bmiHeader.biWidth = bitmapWidth;
	bitmapinfo.bmiHeader.biHeight = -bitmapHeight;
	bitmapinfo.bmiHeader.biPlanes = 1;
	bitmapinfo.bmiHeader.biBitCount = 32;
	bitmapinfo.bmiHeader.biCompression = BI_RGB;
	
	int bitmapMemorySize = (bitmapWidth * bitmapHeight) * bytesPerPixel;
	
	bitmapMemory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
	
	renderBitmap(120, 0);
}

internal void 
updateWindow(HDC deviceContext, RECT *rect, int x, int y, int width, int height)
{
	
	int windowWidth = rect->right - rect->left;
	int windowHeight = rect->bottom - rect->top;
	
	StretchDIBits(
		deviceContext,
		// x, y, width, height,
		// x, y, width, height,
		0, 0, bitmapWidth, bitmapHeight,
		0, 0, windowWidth, windowHeight,
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
			RECT clientRect;
			GetClientRect(windowHandle, &clientRect);
			updateWindow(deviceContext, &clientRect, x, y, width, height);
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
			MSG msg;
			int xOffSet = 0;
			int yOffSet = 0;
			while(running)
			{
				//GetMessageA can give you -1 as well if error occurs so exit
				// when msgResult <= 0
				// BOOL msgResult = GetMessageA(&msg, 0, 0, 0);
				// GetMessageA is ineffiecent call for windows switch to PeekMessageA
				while(PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
				{
					if(msg.message == WM_QUIT)
					{
						running = false;
					}
					TranslateMessage(&msg);
					DispatchMessageA(&msg);
				}
				
				renderBitmap(xOffSet, yOffSet);
				HDC deviceContext = GetDC(windowHandle);
				RECT clientRect;
				GetClientRect(windowHandle, &clientRect);
				int windowWidth = clientRect.right - clientRect.left;
				int windowHeight = clientRect.bottom - clientRect.top;
				updateWindow(deviceContext, &clientRect, 0, 0, windowWidth, windowHeight);
				ReleaseDC( windowHandle, deviceContext);
				
				// ++xOffSet;
				++yOffSet;
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
