// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

//#include <stdio.h>
#include <tchar.h>



// TODO: 在此处引用程序需要的其他头文件
#include <unordered_map>
#include <memory>
#include <map>
#include <io.h>
#include <fcntl.h>
#include <sstream>
#include <algorithm>

//namespace DX
//{
//	// Helper class for COM exceptions
//	class com_exception : public std::exception
//	{
//	public:
//		com_exception(HRESULT hr) : result(hr) {}
//
//		virtual const char* what() const override
//		{
//			static char s_str[64] = {};
//			sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
//			return s_str;
//		}
//
//	private:
//		HRESULT result;
//	};
//
//	// Helper utility converts D3D API failures into exceptions.
//	inline void ThrowIfFailed(HRESULT hr)
//	{
//		if (FAILED(hr))
//		{
//			throw com_exception(hr);
//		}
//	};
//
//	// Helper class for safely use HANDLE
//	class SafeHandle {
//	private:
//		HANDLE m_Handle;
//	public:
//		SafeHandle(HANDLE h) :m_Handle(h) {};
//		~SafeHandle() { CloseHandle(m_Handle); m_Handle = NULL; }
//		operator HANDLE() { return m_Handle; }
//		HANDLE& operator=(HANDLE oth) { m_Handle = oth; return m_Handle; }
//	};
//}
