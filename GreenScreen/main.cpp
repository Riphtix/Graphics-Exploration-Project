// Simple basecode showing how to create a window and attatch a d3d11surface
#define GATEWARE_ENABLE_CORE // All libraries need this
#define GATEWARE_ENABLE_SYSTEM // Graphics libs require system level libraries
#define GATEWARE_ENABLE_GRAPHICS // Enables all Graphics Libraries
// Ignore some GRAPHICS libraries we aren't going to use
#define GATEWARE_DISABLE_GDIRECTX12SURFACE // we have another template for this
#define GATEWARE_DISABLE_GRASTERSURFACE // we have another template for this
#define GATEWARE_DISABLE_GOPENGLSURFACE // we have another template for this
#define GATEWARE_DISABLE_GVULKANSURFACE // we have another template for this
// With what we want & what we don't defined we can include the API
#include "../Gateware/Gateware.h"
#include "triangle.h" // example rendering code (not Gateware code!)
// open some namespaces to compact the code a bit
using namespace GW;
using namespace CORE;
using namespace SYSTEM;
using namespace GRAPHICS;
// lets pop a window and use D3D11 to clear to a green screen
int main()
{
	GWindow win;
	GEventReceiver msgs;
	GDirectX11Surface d3d11;
	if (+win.Create(0, 0, 800, 600, GWindowStyle::WINDOWEDBORDERED))
	{
		float clr[] = { 57/255.0f, 1.0f, 20/255.0f, 1 }; // start with a neon green
		msgs.Create(win, [&]() {
			if (+msgs.Find(GWindow::Events::RESIZE, true))
				clr[2] += 0.01f; // move towards a cyan as they resize
			});
		if (+d3d11.Create(win, 0))
		{
			Triangle tri(win, d3d11);
			while (+win.ProcessWindowEvents())
			{
				IDXGISwapChain* swap;
				ID3D11DeviceContext* con;
				ID3D11RenderTargetView* view;
				if (+d3d11.GetImmediateContext((void**)&con) &&
					+d3d11.GetRenderTargetView((void**)&view) &&
					+d3d11.GetSwapchain((void**)&swap))
				{
					con->ClearRenderTargetView(view, clr);
					tri.Render();
					swap->Present(1, 0);
					// release incremented COM reference counts
					swap->Release();
					view->Release();
					con->Release();
				}
			}
		}
	}
	return 0; // that's all folks
}