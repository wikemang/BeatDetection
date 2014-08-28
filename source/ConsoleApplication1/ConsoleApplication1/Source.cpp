#include <stdio.h>
#include <iostream>
#include <string>
#include <dshow.h>
#include <cstdio>

using namespace std;
// For IID_IGraphBuilder, IID_IMediaControl, IID_IMediaEvent
#pragma comment(lib, "strmiids.lib") 

// Obviously change this to point to a valid mp3 file.
const wchar_t* filePath = L"../../../noise.mp3";

int main()
{
	IGraphBuilder *pGraph = NULL;
	IMediaControl *pControl = NULL;
	IMediaEvent   *pEvent = NULL;

	// Initialize the COM library.
	HRESULT hr = ::CoInitialize(NULL);
	if (FAILED(hr))
	{
		::printf("ERROR - Could not initialize COM library");
		return 0;
	}

	// Create the filter graph manager and query for interfaces.
	hr = ::CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder, (void **)&pGraph);
	if (FAILED(hr))
	{
		::printf("ERROR - Could not create the Filter Graph Manager.");
		return 0;
	}

	hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);

	// Build the graph.
	cout << "start?" << endl;
	hr = pGraph->RenderFile(filePath, NULL);
	if (SUCCEEDED(hr))
	{
		// Run the graph.
		hr = pControl->Run();
		if (SUCCEEDED(hr))
		{
			// Wait for completion.
			while (true){
				cout << "-";
				long evCode;
				HRESULT asdf = pEvent->WaitForCompletion(100, &evCode);
				if (asdf == VFW_E_WRONG_STATE)
					break;
			}
			cout << "end??" << endl;

			// Note: Do not use INFINITE in a real application, because it
			// can block indefinitely.
		}
	}
	// Clean up in reverse order.
	pEvent->Release();
	pControl->Release();
	pGraph->Release();
	::CoUninitialize();
}