﻿
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#include "asd.Texture2D_Imp.h"
#include "../asd.Graphics_Imp.h"

#define Z_SOLO
#include <png.h>

#include <pngstruct.h>
#include <pnginfo.h>

#ifdef _WIN64

#if _DEBUG
#pragma comment(lib, "x64/Debug/libpng16_staticd.lib")
#pragma comment(lib, "x64/Debug/zlibstaticd.lib")
#else
#pragma comment(lib, "x64/Release/libpng16_static.lib")
#pragma comment(lib, "x64/Release/zlibstatic.lib")
#endif

#else

#if _DEBUG
#pragma comment(lib, "x86/Debug/libpng16_staticd.lib")
#pragma comment(lib, "x86/Debug/zlibstaticd.lib")
#else
#pragma comment(lib, "x86/Release/libpng16_static.lib")
#pragma comment(lib, "x86/Release/zlibstatic.lib")
#endif

#endif

namespace asd
{

bool Texture2D_Imp::InternalLoad(void* data, int32_t size, bool rev)
{
	InternalUnload();

	void* imagedata = nullptr;
	auto g = (Graphics_Imp*)GetGraphics();
	if (ImageHelper::LoadPNGImage(data, size, rev, m_internalTextureWidth, m_internalTextureHeight, m_internalTextureData, g->GetLog()))
	{
		return true;
	}

	return false;
}

void Texture2D_Imp::InternalUnload()
{
	m_internalTextureWidth = 0;
	m_internalTextureHeight = 0;
	std::vector<uint8_t>().swap(m_internalTextureData);
}

Texture2D_Imp::Texture2D_Imp(Graphics* graphics) : DeviceObject(graphics), m_internalTextureWidth(0), m_internalTextureHeight(0)
{
	m_type = TextureClassType::Texture2D;
	m_loadState = LoadState::Loading;
}

Texture2D_Imp::Texture2D_Imp(Graphics* graphics, ar::Texture2D* rhi, Vector2DI size, TextureFormat format)
	: DeviceObject(graphics), m_internalTextureWidth(0), m_internalTextureHeight(0), rhi(rhi)
{
	m_type = TextureClassType::Texture2D;
	m_format = format;
	m_size = size;
	m_resource.resize(size.X * size.Y * ImageHelper::GetPitch(m_format));

	auto g = (Graphics_Imp*)GetGraphics();
	g->IncVRAM(ImageHelper::GetVRAMSize(GetFormat(), GetSize().X, GetSize().Y));
	m_loadState = LoadState::Loaded;
}

Texture2D_Imp::~Texture2D_Imp()
{
	auto g = (Graphics_Imp*)GetGraphics();
	g->Texture2DContainer->Unregister(this);

	g->DecVRAM(ImageHelper::GetVRAMSize(GetFormat(), GetSize().X, GetSize().Y));

	asd::SafeDelete(rhi);
}

Texture2D_Imp* Texture2D_Imp::Create(Graphics_Imp* graphics) { return new Texture2D_Imp(graphics); }

Texture2D_Imp* Texture2D_Imp::Create(Graphics_Imp* graphics, uint8_t* data, int32_t size, bool isEditable, bool isSRGB)
{
	if (size == 0)
		return nullptr;

	auto rhi = ar::Texture2D::Create(graphics->GetRHI());

	if (rhi->Initialize(graphics->GetRHI(), data, size, isEditable, isSRGB))
	{
		return new Texture2D_Imp(graphics, rhi, Vector2DI(rhi->GetWidth(), rhi->GetHeight()), (asd::TextureFormat)rhi->GetFormat());
	}

	asd::SafeDelete(rhi);

	return nullptr;
}

Texture2D_Imp* Texture2D_Imp::Create(Graphics_Imp* graphics, int32_t width, int32_t height, TextureFormat format, void* data)
{
	auto rhi = ar::Texture2D::Create(graphics->GetRHI());

	if (rhi->Initialize(graphics->GetRHI(), width, height, (ar::TextureFormat)format, data, true))
	{
		return new Texture2D_Imp(graphics, rhi, Vector2DI(rhi->GetWidth(), rhi->GetHeight()), (asd::TextureFormat)rhi->GetFormat());
	}

	asd::SafeDelete(rhi);

	return nullptr;
}

bool asd::Texture2D_Imp::Load(uint8_t* data, int32_t size, bool isEditable, bool isSRGB)
{
	auto rhi = ar::Texture2D::Create(((Graphics_Imp*)GetGraphics())->GetRHI());

	if (rhi->Initialize(((Graphics_Imp*)GetGraphics())->GetRHI(), data, size, isEditable, isSRGB))
	{
		this->rhi = rhi; 
		m_format = (asd::TextureFormat)rhi->GetFormat();
		m_size = Vector2DI(rhi->GetWidth(), rhi->GetHeight());
		m_resource.resize(m_size.X * m_size.Y * ImageHelper::GetPitch(m_format));

		auto g = (Graphics_Imp*)GetGraphics();
		g->IncVRAM(ImageHelper::GetVRAMSize(GetFormat(), GetSize().X, GetSize().Y));
		m_loadState = LoadState::Loaded;
		return true;
	}

	m_loadState = LoadState::Failed;
	return false;

	asd::SafeDelete(rhi);
}

bool Texture2D_Imp::Save(const achar* path)
{
	std::vector<ar::Color> dst;
	int32_t width;
	int32_t height;

	if (rhi->Save(dst, width, height))
	{
		ar::ImageHelper::SavePNG(path, width, height, dst.data());
		return true;
	}
	return false;
}

bool Texture2D_Imp::Lock(TextureLockInfomation* info) { return rhi->Lock((ar::TextureLockInfomation*)info); }

void Texture2D_Imp::Unlock() { rhi->Unlock(); }

void Texture2D_Imp::Reload(void* data, int32_t size)
{
	auto g = (Graphics_Imp*)GetGraphics();
	g->DecVRAM(ImageHelper::GetVRAMSize(GetFormat(), GetSize().X, GetSize().Y));

	asd::SafeDelete(rhi);

	rhi = ar::Texture2D::Create(g->GetRHI());

	if (rhi->Initialize(g->GetRHI(), data, size, false, this->m_format == TextureFormat::R8G8B8A8_UNORM_SRGB))
	{
	}

	g->IncVRAM(ImageHelper::GetVRAMSize(GetFormat(), GetSize().X, GetSize().Y));
}

} // namespace asd