//***************************************************************************************
// d3dUtil.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#ifndef D3DUTIL_H
#define D3DUTIL_H

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#define _XM_NO_INTRINSICS_
 
#include <d3d11.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <cassert>
#include <ctime>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include "MathHelper.h"

//---------------------------------------------------------------------------------------
// Simple d3d error checker for book demos.
//---------------------------------------------------------------------------------------

#if defined(DEBUG) | defined(_DEBUG)
	#ifndef HR
	#define HR(x)                                              \
	{                                                          \
		HRESULT hr = (x);                                      \
		if(FAILED(hr))                                         \
		{                                                      \
			char buff[256] = { 0 };\
			sprintf_s(buff, 256, "%s, %d, %d", __FILE__, __LINE__, x);\
			OutputDebugStringA(buff); \
		}                                                      \
	}
	#endif

#else
	#ifndef HR
	#define HR(x) (x)
	#endif
#endif 


//---------------------------------------------------------------------------------------
// Convenience macro for releasing COM objects.
//---------------------------------------------------------------------------------------

#define ReleaseCOM(x) { if(x){ x->Release(); x = 0; } }

//---------------------------------------------------------------------------------------
// Convenience macro for deleting objects.
//---------------------------------------------------------------------------------------

#define SafeDelete(x) { delete x; x = 0; }

//---------------------------------------------------------------------------------------
// Utility classes.
//---------------------------------------------------------------------------------------

class TextHelper
{
public:

	template<typename T>
	static std::wstring ToString(const T& s)
	{
		std::wostringstream oss;
		oss << s;

		return oss.str();
	}

	template<typename T>
	static T FromString(const std::wstring& s)
	{
		T x;
		std::wistringstream iss(s);
		iss >> x;

		return x;
	}
};

// Order: left, right, bottom, top, near, far.
void ExtractFrustumPlanes(DirectX::XMFLOAT4 planes[6], DirectX::CXMMATRIX M);


// #define XMGLOBALCONST extern CONST __declspec(selectany)
//   1. extern so there is only one copy of the variable, and not a separate
//      private copy in each .obj.
//   2. __declspec(selectany) so that the compiler does not complain about
//      multiple definitions in a .cpp file (it can pick anyone and discard 
//      the rest because they are constant--all the same).

namespace Colors
{
	XMGLOBALCONST DirectX::XMVECTORF32 White     = {1.0f, 1.0f, 1.0f, 1.0f};
	XMGLOBALCONST DirectX::XMVECTORF32 Black     = {0.0f, 0.0f, 0.0f, 1.0f};
	XMGLOBALCONST DirectX::XMVECTORF32 Red       = {1.0f, 0.0f, 0.0f, 1.0f};
	XMGLOBALCONST DirectX::XMVECTORF32 Green     = {0.0f, 1.0f, 0.0f, 1.0f};
	XMGLOBALCONST DirectX::XMVECTORF32 Blue      = {0.0f, 0.0f, 1.0f, 1.0f};
	XMGLOBALCONST DirectX::XMVECTORF32 Yellow    = {1.0f, 1.0f, 0.0f, 1.0f};
	XMGLOBALCONST DirectX::XMVECTORF32 Cyan      = {0.0f, 1.0f, 1.0f, 1.0f};
	XMGLOBALCONST DirectX::XMVECTORF32 Magenta   = {1.0f, 0.0f, 1.0f, 1.0f};

	XMGLOBALCONST DirectX::XMVECTORF32 Silver    = {0.75f, 0.75f, 0.75f, 1.0f};
	XMGLOBALCONST DirectX::XMVECTORF32 LightSteelBlue = {0.69f, 0.77f, 0.87f, 1.0f};
}

#endif // D3DUTIL_H