#pragma once

#ifndef PCH_H
#define PCH_H

//	Windows.hの無駄削除
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 
#endif

#define NOMINMAX
#include <windows.h>

// DirectX12
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")


// STL
#include<vector>
#include<array>
#include<queue>
#include<map>
#include<unordered_map>
#include<set>
#include<unordered_set>

#include<string>
#include<string_view>
#include<filesystem>

#include<memory>
#include<optional>
#include<variant>
#include<concepts>
#include<coroutine>

#include<algorithm>
#include<functional>
#include<numeric>
#include<tuple>
#include<span>
#include<ranges>
#include<initializer_list>

#include<thread>
#include<mutex>
#include<atomic>

#include<iostream>
#include<sstream>
#include<fstream>



//	ホットリロード実装する可能性あるので前提で組みます。
#ifdef MYENGINE_AS_DLL
//	Dllの時
#ifdef MYENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif
#else
//	libの時は何もしない
#define ENGINE_API	
#endif





namespace Ecse
{

}

//	STL


#endif //PCH_H
