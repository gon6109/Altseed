﻿
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#include "asd.Window_Imp.h"
#include "asd.Cursor_Imp.h"

#include "../Log/asd.Log_Imp.h"

#ifdef _WIN32
#include "asd.Window_Imp_Win.h"
#else
#include "asd.Window_Imp_X11.h"
#endif

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace asd {
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

static void CallbackOnFocus(GLFWwindow* window, int b)
{
	auto w = (Window_Imp*)glfwGetWindowUserPointer(window);
	if (b == GL_TRUE)
	{
		auto onFocused = w->OnFocused;

		if (onFocused)
		{
			onFocused();
		}
	}
	else
	{

	}
}

Window_Imp::Window_Imp(Log* logger)
	: m_logger(logger)
{

}

Window_Imp::~Window_Imp()
{
	SafeRelease(currentCursor);
	SafeDelete(window);
}

Window_Imp* Window_Imp::Create(
	int32_t width,
	int32_t height,
	const achar* title,
	Log* logger,
	WindowPositionType windowPositionType,
	GraphicsDeviceType graphicsDeviceType,
	ColorSpaceType colorSpaceType,
	bool isFullScreen)
{
	if (logger != nullptr) logger->WriteHeading("ウインドウ");

	auto ret = new Window_Imp(logger);
	auto window = ap::Window::Create();

	ap::WindowInitializationParameter initParam;
	initParam.WindowWidth = width;
	initParam.WindowHeight = height;
	initParam.Title = std::u16string(title);
	initParam.WindowPosition = (ap::WindowPositionType)windowPositionType;
	initParam.ColorSpace = (ap::ColorSpaceType)colorSpaceType;
	initParam.GraphicsDevice = (ap::GraphicsDeviceType)graphicsDeviceType;
	initParam.IsFullscreenMode = isFullScreen;

	if (ret != nullptr && window != nullptr)
	{
		auto errorCode = window->Initialize(initParam);
		if (errorCode == ap::ErrorCode::OK)
		{
			ret->window = window;
			ret->m_size.X = width;
			ret->m_size.Y = height;

			auto glfwWindow = (GLFWwindow*)window->GetNativeWindow();
			glfwSetWindowUserPointer(glfwWindow, ret);
			glfwSetWindowFocusCallback(glfwWindow, CallbackOnFocus);

			if (logger != nullptr) logger->WriteLine("ウインドウ作成成功");
		}
		else
		{
			if (logger != nullptr) logger->WriteLine("ウインドウ作成失敗");
			goto End;
		}
	}
	else
	{
		if (logger != nullptr) logger->WriteLine("ウインドウ作成失敗");
		goto End;
	}

	return ret;

End:;
	SafeRelease(ret);
	SafeDelete(window);
	return nullptr;
}

bool Window_Imp::DoEvent()
{
	return window->DoEvent();
}

void Window_Imp::SetTitle(const achar* title)
{
	window->SetTitle(title);
}

void Window_Imp::Close()
{
	window->Close();
}

void* Window_Imp::GetWindowHandle() const
{
	return window->GetHandle();
}

void Window_Imp::SetSize(Vector2DI size)
{
	window->SetSize(size.X, size.Y);
}

void Window_Imp::SetCursor(Cursor* cursor)
{
	auto glfwWindow = (GLFWwindow*)window->GetNativeWindow();

	if (cursor == nullptr)
	{
		glfwSetCursor(glfwWindow, nullptr);
		SafeRelease(currentCursor);
		return;
	}

	auto c = (Cursor_Imp*) cursor;
	glfwSetCursor(glfwWindow, c->GetNative());

	SafeSubstitute(currentCursor, cursor);
}

const achar* Window_Imp::GetClipboardString()
{
	auto glfwWindow = (GLFWwindow*)window->GetNativeWindow();

	auto s = glfwGetClipboardString(glfwWindow);
	static achar temp[260];

	if (s == nullptr)
	{
		temp[0] = 0;
		return temp;
	}

	std::vector<int16_t> dst;
	int32_t len = Utf8ToUtf16(dst, (const int8_t*)s);

	int32_t ind = 0;
	for (ind = 0; ind < Min(len, 260); ind++)
	{
		temp[ind] = dst[ind];
	}

	if (ind < 260)
	{
		temp[ind] = 0;
	}

	temp[259] = 0;

	return temp;
}

void Window_Imp::SetClipboardString(const achar* s)
{
	auto glfwWindow = (GLFWwindow*)window->GetNativeWindow();

	std::vector<int8_t> dst;

	Utf16ToUtf8(dst, (int16_t*)s);

	glfwSetClipboardString(glfwWindow, (const char*)dst.data());
}

ap::Window* Window_Imp::GetWindow()
{
	return window;
}

}