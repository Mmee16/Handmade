#include <windows.h>
#include <stdint.h>

#define internal static
#define global_var static

struct Bitmap_buffer
{
	void* memory;
	BITMAPINFO info;
	int width;
	int height;
	int pitch;
	// bitmapMemory stores the info reg the color of each pixel
	// So the size of bitmapMemory = #(pixels) * 4 (- 4 = 3 rgb colors 3 bytes + 1 byte for padding)
	int bytesPerPixel;
};

struct Win32_Dimension
{
	int width;
	int height;
};

global_var bool running;
global_var Bitmap_buffer globalBuffer;

internal void RenderWeirdGradient(Bitmap_buffer buffer, int xOffSet, int yOffSet)
{
	uint8_t *row = (uint8_t *)(buffer.memory);
	
	for(int y=0; y<buffer.height; y++)
	{
		uint32_t *pixel = (uint32_t *)row;
		for(int x=0; x<buffer.width; x++)
		{
			uint8_t blue = (x+xOffSet);
			uint8_t green = (y+yOffSet);
			// Memory 	BB GG RR xx
			// Register xx RR GG BB - due to little endian
			*pixel++ = ((green << 8) | blue);
		}
		row+=buffer.pitch;
	}
}

Win32_Dimension getWindowDimension(HWND windowHandle)
{
	Win32_Dimension dimension;
	RECT clientRect;
	// This gives the actual part where you can draw into - 
	// Where as GetWindowRect gives total rect including scrollbar,buttons,...
	GetClientRect(windowHandle, &clientRect);
	dimension.width = clientRect.right - clientRect.left;
	dimension.height = clientRect.bottom - clientRect.top;
	return dimension;
}

//DIB - Device independent Bitmap
// This function Creates a new bitmap when ever window size changes
/** While creating a new bitmap
Either you can first free the old buffer and create a new one or
You can create a new buffer then free the old one if this fails we can
fall back to first one
**/
internal void
ResizeDIBSection(Bitmap_buffer  *buffer, int width, int height)
{
	if(buffer->memory)
	{
		VirtualFree(buffer->memory, 0, MEM_RELEASE);
	}
	
	buffer->width = width;
	buffer->height = height;
	buffer->bytesPerPixel = 4;

	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = buffer->width;
	buffer->info.bmiHeader.biHeight = -buffer->height;
	buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;
	buffer->info.bmiHeader.biCompression = BI_RGB;
	
	int bitmapMemorySize = (buffer->width * buffer->height) * buffer->bytesPerPixel;
	
	buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
	
	buffer->pitch = buffer->width * buffer->bytesPerPixel;

	//RenderWeirdGradient(globalBuffer, 120, 0);
}

internal void 
updateWindow(HDC deviceContext, Win32_Dimension dimension, Bitmap_buffer buffer,
			int x, int y, int width, int height)//  The last 4 params are not used
{
	
	StretchDIBits(
		deviceContext,
		// x, y, width, height,
		// x, y, width, height,
		0, 0, dimension.width, dimension.height,
		0, 0, buffer.width, buffer.height,
		buffer.memory,
		&buffer.info,
		DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK MainCallBack(
  HWND windowHandle,
  UINT message,
  WPARAM wParam,
  LPARAM lParam
)
{
	switch(message)
	{
		// This msg is sent when window size changes
		case WM_SIZE:
		{
			break;
		}
		// This message is posted when window is closed (X button, alt+f4,...)
		case WM_CLOSE:
		{
			running = false;
			//OutputDebugStringA("WM_CLOSE\n");
			break;
		}
		case WM_DESTROY:
		{
			// Something destroyed the window handle the recreation of the window
			//OutputDebugStringA("Something destroyed the window recreate it maybe\n");
			break;
		}
		case WM_ACTIVATEAPP:
		{
			//OutputDebugStringA("WM_ACTIVATEAPP\n");
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC deviceContext = BeginPaint(windowHandle, &paint);
			Win32_Dimension dimension = getWindowDimension(windowHandle);
			updateWindow(deviceContext, dimension, globalBuffer,0 ,0, dimension.width, dimension.height);
			EndPaint(windowHandle, &paint);
			OutputDebugStringA("WM_PAINT");
			break;
		}
		default:
		{
			//OutputDebugStringA("Default\n");
			return DefWindowProc( windowHandle, message, wParam, lParam);
		}
	}
	return 0;
}

int 
WinMain(
  HINSTANCE hInstance, // Can be fetched from GetModuleHandle() Function,
  HINSTANCE hPrevInstance,
  LPSTR     lpCmdLine,
  int       nShowCmd)
{
	WNDCLASSA windowClass = {};
	windowClass.style = CS_HREDRAW|CS_HREDRAW|CS_OWNDC ; 
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
			// CS_OWNDC gives one DC per window so we can get it once and use it forever
			HDC deviceContext = GetDC(windowHandle);

			Win32_Dimension dimension = getWindowDimension(windowHandle);
			// When window size changes we want to redraw our things based on the new window size
			ResizeDIBSection(&globalBuffer, dimension.width, dimension.height);

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
				
				RenderWeirdGradient(globalBuffer, xOffSet, yOffSet);

				Win32_Dimension dimension = getWindowDimension(windowHandle);
				updateWindow(deviceContext, dimension, globalBuffer, 0, 0, dimension.width, dimension.height);
				
				++xOffSet;
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
