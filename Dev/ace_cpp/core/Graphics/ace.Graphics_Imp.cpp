﻿
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#include <vector>
#include "ace.Graphics_Imp.h"
#include "ace.RenderingThread.h"

#include "../Log/ace.Log.h"
#include "../IO/ace.File_Imp.h"

#include "Resource/ace.VertexBuffer_Imp.h"
#include "Resource/ace.IndexBuffer_Imp.h"
#include "Resource/ace.NativeShader_Imp.h"
#include "Resource/ace.Effect_Imp.h"

#include "Resource/ace.Shader2D_Imp.h"
#include "Resource/ace.Material2D_Imp.h"

#include "Resource/ace.Shader3D_Imp.h"
#include "Resource/ace.Material3D_Imp.h"

#include "Resource/ace.Chip2D_Imp.h"

#include "Resource/ace.Effect_Imp.h"

#include "Resource/ace.ShaderCache.h"
#include "Resource/ace.MaterialPropertyBlock_Imp.h"

#include "3D/Resource/ace.Mesh_Imp.h"
#include "3D/Resource/ace.Deformer_Imp.h"
#include "3D/Resource/ace.Model_Imp.h"
#include "3D/Resource/ace.MassModel_Imp.h"
#include "3D/Resource/ace.Terrain3D_Imp.h"

#include <Graphics/3D/ace.Model_IO.h>
#include <Graphics/3D/ace.MassModel_IO.h>

#if _WIN32
#include "Platform/DX11/ace.Graphics_Imp_DX11.h"
#endif

#include "Platform/GL/ace.Graphics_Imp_GL.h"

#define Z_SOLO
#include <png.h>
#include <pngstruct.h>
#include <pnginfo.h>

#if _WIN32
#if _DEBUG
#pragma comment(lib,"libpng16.Debug.lib")
#pragma comment(lib,"zlib.Debug.lib")
#else
#pragma comment(lib,"libpng16.Release.lib")
#pragma comment(lib,"zlib.Release.lib")
#endif
#endif

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#if _WIN32
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#else
#include <sys/stat.h>
#endif

static void CreateShaderCacheDirectory()
{
	const char* shaderCacheDirectory = "ShaderCache";
#if _WIN32
	if (!PathIsDirectoryA(shaderCacheDirectory))
	{
		CreateDirectoryA(shaderCacheDirectory, NULL);
	}
#else
	mkdir(shaderCacheDirectory,
		S_IRUSR | S_IWUSR | S_IXUSR |
		S_IRGRP | S_IWGRP | S_IXGRP |
		S_IROTH | S_IXOTH | S_IXOTH);
#endif
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace ace {

	class EffectLoader
		: public Effekseer::EffectLoader
	{
	public:
		bool Load(const EFK_CHAR* path, void*& data, int32_t& size)
		{
#if _WIN32
			auto fp = _wfopen((const achar*)path, L"rb");
			if (fp == nullptr) return false;
#else
			auto fp = fopen(ToUtf8String((const achar*)path).c_str(), "rb");
			if (fp == nullptr) return false;
#endif
			fseek(fp, 0, SEEK_END);
			size = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			data = new uint8_t[size];
			fread(data, 1, size, fp);
			fclose(fp);

			return true;
		}

		void Unload(void* data, int32_t size)
		{
			auto d = (uint8_t*) data;
			SafeDeleteArray(d);
		}
	};

	static void GetParentDir(EFK_CHAR* dst, const EFK_CHAR* src)
	{
		int i, last = -1;
		for (i = 0; src[i] != L'\0'; i++)
		{
			if (src[i] == L'/' || src[i] == L'\\')
				last = i;
		}
		if (last >= 0)
		{
			memcpy(dst, src, last * sizeof(EFK_CHAR));
			dst[last] = L'\0';
		}
		else
		{
			dst[0] = L'\0';
		}
	}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
static void PngReadData(png_structp png_ptr, png_bytep data, png_size_t length)
{
	auto d = (uint8_t**) png_get_io_ptr(png_ptr);

	memcpy(data, *d, length);
	(*d) += length;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ImageHelper::SavePNGImage(const achar* filepath, int32_t width, int32_t height, void* data, bool rev)
{
	png_bytep raw1D;
	png_bytepp raw2D;

	/* 構造体確保 */
#if _WIN32
	FILE *fp = _wfopen(filepath, L"wb");
#else
	FILE *fp = fopen(ToUtf8String(filepath).c_str(), "wb");
#endif

	if (fp == nullptr) return;

	png_structp pp = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop ip = png_create_info_struct(pp);

	/* 書き込み準備 */
	png_init_io(pp, fp);
	png_set_IHDR(pp, ip, width, height,
		8, /* 8bit以外にするなら変える */
		PNG_COLOR_TYPE_RGBA, /* RGBA以外にするなら変える */
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	/* ピクセル領域確保 */
	raw1D = (png_bytep) malloc(height * png_get_rowbytes(pp, ip));
	raw2D = (png_bytepp) malloc(height * sizeof(png_bytep));
	for (int32_t i = 0; i < height; i++)
	{
		raw2D[i] = &raw1D[i*png_get_rowbytes(pp, ip)];
	}

	memcpy((void*) raw1D, data, width * height * 4);

	/* 上下反転 */
	if (rev)
	{
		for (int32_t i = 0; i < height / 2; i++)
		{
			png_bytep swp = raw2D[i];
			raw2D[i] = raw2D[height - i - 1];
			raw2D[height - i - 1] = swp;
		}
	}

	/* 書き込み */
	png_write_info(pp, ip);
	png_write_image(pp, raw2D);
	png_write_end(pp, ip);

	/* 開放 */
	png_destroy_write_struct(&pp, &ip);
	fclose(fp);
	free(raw1D);
	free(raw2D);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
bool ImageHelper::LoadPNGImage(void* data, int32_t size, bool rev, int32_t& imagewidth, int32_t& imageheight, std::vector<uint8_t>& imagedst)
{
	imagewidth = 0;
	imageheight = 0;
	imagedst.clear();

	uint8_t* data_ = (uint8_t*) data;

	/* pngアクセス構造体を作成 */
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	/* リードコールバック関数指定 */
	png_set_read_fn(png, &data_, &PngReadData);

	/* png画像情報構造体を作成 */
	png_infop png_info = png_create_info_struct(png);

	/* エラーハンドリング */
	if (setjmp(png_jmpbuf(png)))
	{
		png_destroy_read_struct(&png, &png_info, NULL);
		return false;
	}

	/* IHDRチャンク情報を取得 */
	png_read_info(png, png_info);

	/* RGBA8888フォーマットに変換する */
	if (png_info->bit_depth < 8)
	{
		png_set_packing(png);
	}
	else if (png_info->bit_depth == 16)
	{
		png_set_strip_16(png);
	}

	uint32_t pixelBytes = 4;
	switch (png_info->color_type)
	{
	case PNG_COLOR_TYPE_PALETTE:
		{
			png_set_palette_to_rgb(png);

			png_bytep trans_alpha = NULL;
			int num_trans = 0;
			png_color_16p trans_color = NULL;

			png_get_tRNS(png, png_info, &trans_alpha, &num_trans, &trans_color);
			if (trans_alpha != NULL)
			{
				pixelBytes = 4;
			}
			else
			{
				pixelBytes = 3;
			}
		}
		break;
	case PNG_COLOR_TYPE_GRAY:
		png_set_expand_gray_1_2_4_to_8(png);
		pixelBytes = 3;
		break;
	case PNG_COLOR_TYPE_RGB:
		pixelBytes = 3;
		break;
	case PNG_COLOR_TYPE_RGBA:
		break;
	}

	uint8_t* image = new uint8_t[png_info->width * png_info->height * pixelBytes];
	uint32_t pitch = png_info->width * pixelBytes;

	// 読み込み
	if (rev)
	{
		for (uint32_t i = 0; i < png_info->height; i++)
		{
			png_read_row(png, &image[(png_info->height - 1 - i) * pitch], NULL);
		}
	}
	else
	{
		for (uint32_t i = 0; i < png_info->height; i++)
		{
			png_read_row(png, &image[i * pitch], NULL);
		}
	}

	imagewidth = png_info->width;
	imageheight = png_info->height;
	imagedst.resize(imagewidth * imageheight * 4);
	auto imagedst_ = imagedst.data();

	if (pixelBytes == 4)
	{
		memcpy(imagedst_, image, imagewidth * imageheight * 4);
	}
	else
	{
		for (int32_t y = 0; y < imageheight; y++)
		{
			for (int32_t x = 0; x < imagewidth; x++)
			{
				auto src = (x + y * imagewidth) * 3;
				auto dst = (x + y * imagewidth) * 4;
				imagedst_[dst + 0] = image[src + 0];
				imagedst_[dst + 1] = image[src + 1];
				imagedst_[dst + 2] = image[src + 2];
				imagedst_[dst + 3] = 255;
			}
		}
	}

	delete[] image;
	png_destroy_read_struct(&png, &png_info, NULL);

	return true;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
int32_t ImageHelper::GetPitch(TextureFormat format)
{
	if (format == TextureFormat::R8G8B8A8_UNORM) return 4;
	if (format == TextureFormat::R32G32B32A32_FLOAT) return 4 * 4;
	if (format == TextureFormat::R8G8B8A8_UNORM_SRGB) return 4;
	if (format == TextureFormat::R16G16_FLOAT) return 2 * 2;
	if (format == TextureFormat::R8_UNORM) return 1;
	return 0;
}

int32_t ImageHelper::GetMipmapCount(int32_t width, int32_t height)
{
	auto ret = 1;
	while (width != 1 || height != 1)
	{
		if (width > 1) width = width >> 1;
		if (height > 1) height = height >> 1;
		ret++;
	}

	return ret;
}

void ImageHelper::GetMipmapSize(int mipmap, int32_t& width, int32_t& height)
{
	for (auto i = 0; i < mipmap; i++)
	{
		if (width > 1) width = width >> 1;
		if (height > 1) height = height >> 1;
	}
}


bool ImageHelper::IsDDS(const void* data, int32_t size)
{
	if (size < 4) return false;

	auto d = (uint8_t*) data;

	if (d[0] != 'D') return false;
	if (d[1] != 'D') return false;
	if (d[2] != 'S') return false;

	return true;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
EffectTextureLoader::EffectTextureLoader(Graphics_Imp* graphics)
	: m_graphics(graphics)
{
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
EffectTextureLoader::~EffectTextureLoader()
{
	assert(m_caches.size() == 0);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void* EffectTextureLoader::Load(const EFK_CHAR* path)
{
	auto key = astring((const achar*) path);
	auto cache = m_caches.find(key);
	if (cache != m_caches.end())
	{
		cache->second.Count++;
		return cache->second.Ptr;
	}

	auto staticFile = m_graphics->GetFile()->CreateStaticFile((const achar*) path);
	if (staticFile.get() == nullptr) return nullptr;

	int32_t imageWidth = 0;
	int32_t imageHeight = 0;
	std::vector<uint8_t> imageDst;
	if (!ImageHelper::LoadPNGImage(staticFile->GetData(), staticFile->GetSize(), IsReversed(), imageWidth, imageHeight, imageDst))
	{
		return nullptr;
	}

	void* img = InternalLoad(m_graphics, imageDst, imageWidth, imageHeight);

	Cache c;
	c.Ptr = img;
	c.Count = 1;
	m_caches[key] = c;
	dataToKey[img] = key;
	return img;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectTextureLoader::Unload(void* data)
{
	InternalUnload(data);

	if (data == nullptr) return;

	auto key = dataToKey[data];
	auto cache = m_caches.find(key);
	cache->second.Count--;

	if (cache->second.Count == 0)
	{
		m_caches.erase(key);
		dataToKey.erase(data);
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Graphics_Imp::AddDeviceObject(DeviceObject* o)
{
	assert(m_deviceObjects.count(o) == 0);
	m_deviceObjects.insert(o);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Graphics_Imp::RemoveDeviceObject(DeviceObject* o)
{
	assert(m_deviceObjects.count(o) == 1);
	m_deviceObjects.erase(o);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Graphics_Imp::ResetDrawState()
{
	SafeRelease(m_vertexBufferPtr);
	SafeRelease(m_indexBufferPtr);
	SafeRelease(m_shaderPtr);
}

void Graphics_Imp::StartRenderingThreadFunc(void* self)
{
	auto self_ = (Graphics_Imp*) self;
	self_->StartRenderingThread();
}

void Graphics_Imp::StartRenderingThread()
{

}

void Graphics_Imp::EndRenderingThreadFunc(void* self)
{
	auto self_ = (Graphics_Imp*) self;
	self_->EndRenderingThread();
}

void Graphics_Imp::EndRenderingThread()
{

}


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Shader2D* Graphics_Imp::CreateShader2D_(const achar* shaderText)
{
	return CreateShader2D_Imp(shaderText);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Graphics_Imp* Graphics_Imp::Create(Window* window, GraphicsDeviceType graphicsDevice, Log* log,File* file, bool isReloadingEnabled, bool isFullScreen)
{
#if _WIN32
	if (graphicsDevice == GraphicsDeviceType::OpenGL)
	{
		return Graphics_Imp_GL::Create(window, log, file,isReloadingEnabled, isFullScreen);
	}
	else
	{
		return Graphics_Imp_DX11::Create(window, log, file,isReloadingEnabled, isFullScreen);
	}
#else
	if (graphicsDevice == GraphicsDeviceType::OpenGL)
	{
		return Graphics_Imp_GL::Create(window, log, file, isReloadingEnabled, isFullScreen);
	}
	else
	{
		return nullptr;
	}
#endif
	return nullptr;
}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Graphics_Imp* Graphics_Imp::Create(void* handle1, void* handle2, int32_t width, int32_t height, GraphicsDeviceType graphicsDevice, Log* log,File* file, bool isReloadingEnabled, bool isFullScreen)
{
#if _WIN32
	if (graphicsDevice == GraphicsDeviceType::OpenGL)
	{
		return Graphics_Imp_DX11::Create((HWND) handle1, width, height, log, file,isReloadingEnabled, isFullScreen);
	}
	else
	{
		return Graphics_Imp_DX11::Create((HWND) handle1, width, height, log, file,isReloadingEnabled, isFullScreen);
	}
#elif __APPLE__
	return nullptr; // not supported
#else
	return Graphics_Imp_GL::Create_X11(handle1, handle2, width, height, log, file,isReloadingEnabled, isFullScreen);
#endif
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Graphics_Imp::Graphics_Imp(Vector2DI size, Log* log,File* file, bool isReloadingEnabled, bool isFullScreen)
	: m_size(size)
	, m_vertexBufferPtr(nullptr)
	, m_indexBufferPtr(nullptr)
	, m_shaderPtr(nullptr)
	, m_log(log)
	, m_file(file)
{
	CreateShaderCacheDirectory();

	Texture2DContainer = std::make_shared<ResourceContainer<Texture2D_Imp>>(file);
	EffectContainer = std::make_shared<ResourceContainer<Effect_Imp>>(file);

	//SafeAddRef(m_log);
	m_resourceContainer = new GraphicsResourceContainer(m_file);
	m_renderingThread = std::make_shared<RenderingThread>();
	
	m_effectSetting = Effekseer::Setting::Create();
	m_effectSetting->SetCoordinateSystem(Effekseer::eCoordinateSystem::COORDINATE_SYSTEM_RH);
	m_effectSetting->SetEffectLoader(new EffectLoader());

	m_shaderCache = new ShaderCache(this);

	for (auto i = 0; i < MaxTextureCount; i++)
	{
		currentState.textureFilterTypes[i] = TextureFilterType::Nearest;
		currentState.textureWrapTypes[i] = TextureWrapType::Clamp;
		nextState.textureFilterTypes[i] = TextureFilterType::Nearest;
		nextState.textureWrapTypes[i] = TextureWrapType::Clamp;

		currentState.textureFilterTypes_vs[i] = TextureFilterType::Nearest;
		currentState.textureWrapTypes_vs[i] = TextureWrapType::Clamp;
		nextState.textureFilterTypes_vs[i] = TextureFilterType::Nearest;
		nextState.textureWrapTypes_vs[i] = TextureWrapType::Clamp;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Graphics_Imp::~Graphics_Imp()
{
	SafeDelete(m_shaderCache);

	SafeRelease(m_vertexBufferPtr);
	SafeRelease(m_indexBufferPtr);
	SafeRelease(m_shaderPtr);

	SafeDelete(m_resourceContainer);

	SafeRelease(m_effectSetting);
	//SafeRelease(m_log);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Texture2D_Imp* Graphics_Imp::CreateTexture2D_Imp(const achar* path)
{
	auto ret = Texture2DContainer->TryLoad(path, [this](uint8_t* data, int32_t size) -> Texture2D_Imp*
	{
		return CreateTexture2D_Imp_Internal(this, data, size);
	});

	return ret;
}

Texture2D_Imp* Graphics_Imp::CreateTexture2DAsRawData_Imp(const achar* path)
{
	auto ret = Texture2DContainer->TryLoad(path, [this](uint8_t* data, int32_t size) -> Texture2D_Imp*
	{
		return CreateTexture2DAsRawData_Imp_Internal(this, data, size);
	});

	return ret;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Texture2D_Imp* Graphics_Imp::CreateEmptyTexture2D_Imp(int32_t width, int32_t height, TextureFormat format)
{
	return CreateEmptyTexture2D_Imp_Internal(this, width, height, format);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Shader2D_Imp* Graphics_Imp::CreateShader2D_Imp(const achar* shaderText)
{
	auto shader = Shader2D_Imp::Create(this,shaderText,ToAString("").c_str(),m_log);

	return shader;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Material2D* Graphics_Imp::CreateMaterial2D_(Shader2D* shader)
{
	auto material = Material2D_Imp::Create((Shader2D_Imp*) shader);
	auto material2d = (Material2D*) material;
	return material2d;
}

Shader3D* Graphics_Imp::CreateShader3D_(const achar* shaderText)
{
	auto shader = Shader3D_Imp::Create(this, shaderText, ToAString("").c_str(), m_log);
	return shader;
}

Material3D* Graphics_Imp::CreateMaterial3D_(Shader3D* shader)
{
	auto material = Material3D_Imp::Create((Shader3D_Imp*) shader);
	auto material3d = (Material3D*) material;
	return material3d;
}

MaterialPropertyBlock* Graphics_Imp::CreateMaterialPropertyBlock_()
{
	auto block = new MaterialPropertyBlock_Imp();
	return block;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Mesh* Graphics_Imp::CreateMesh_()
{
	return Mesh_Imp::Create(this);
}

Deformer* Graphics_Imp::CreateDeformer_()
{
	return new Deformer_Imp();
}

Model* Graphics_Imp::CreateModel_(const achar* path)
{
	{
		auto existing = GetResourceContainer()->Models.Get(path);
		if (existing != nullptr)
		{
			SafeAddRef(existing);
			return existing;
		}
	}

	auto staticFile = GetFile()->CreateStaticFile(path);
	if (staticFile.get() == nullptr) return nullptr;
	
	std::vector<uint8_t> data;
	data.resize(staticFile->GetSize());

	memcpy(data.data(), staticFile->GetData(), staticFile->GetSize());

	auto model = new Model_Imp(this);
	model->Load(this, data, path);

	std::shared_ptr<ModelReloadInformation> info;
	info.reset(new ModelReloadInformation());
	info->ModifiedTime = GetResourceContainer()->GetModifiedTime(path);
	info->Path = path;

	GetResourceContainer()->Models.Regist(path, info, model);
	
	return model;
}

MassModel* Graphics_Imp::CreateMassModelFromModelFile_(const achar* path)
{
	auto staticFile = GetFile()->CreateStaticFile(path);
	if (staticFile.get() == nullptr) return nullptr;

	std::vector<uint8_t> data;
	data.resize(staticFile->GetSize());

	memcpy(data.data(), staticFile->GetData(), staticFile->GetSize());

	Model_IO model_io;
	if (!model_io.Load(data, path))
	{
		return nullptr;
	}

	MassModel_IO massmodel_io;
	if (!massmodel_io.Convert(model_io))
	{
		return nullptr;
	}

	auto massmodel = new MassModel_Imp();

	if (!massmodel->Load(this, massmodel_io))
	{
		SafeDelete(massmodel);
	}

	return massmodel;
}

MassModel* Graphics_Imp::CreateMassModel_(const achar* path)
{
	auto staticFile = GetFile()->CreateStaticFile(path);
	if (staticFile.get() == nullptr) return nullptr;

	std::vector<uint8_t> data;
	data.resize(staticFile->GetSize());

	memcpy(data.data(), staticFile->GetData(), staticFile->GetSize());

	MassModel_IO massmodel_io;
	if (!massmodel_io.Load(data, path))
	{
		return nullptr;
	}

	auto massmodel = new MassModel_Imp();

	if (!massmodel->Load(this, massmodel_io))
	{
		SafeDelete(massmodel);
	}

	return massmodel;
}

Terrain3D* Graphics_Imp::CreateTerrain3D_()
{
	return new Terrain3D_Imp(this);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Effect* Graphics_Imp::CreateEffect_(const achar* path)
{
	auto ret = EffectContainer->TryLoad(path, [this,path](uint8_t* data, int32_t size) -> Effect_Imp*
	{
		EFK_CHAR parentDir[512];	
		GetParentDir(parentDir, (const EFK_CHAR*) path);
	
		auto effect = Effekseer::Effect::Create(m_effectSetting, data, size, 1.0, parentDir);
		if (effect == nullptr) return nullptr;
		return Effect_Imp::CreateEffect(this, effect);
	});

	return ret;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Font* Graphics_Imp::CreateFont_(const achar* path)
{
	{
		auto existing = GetResourceContainer()->Fonts.Get(path);
		if (existing != nullptr)
		{
			SafeAddRef(existing);
			return existing;
		}
	}

	auto affFile = m_file->CreateStaticFile(path);

	if (affFile == nullptr) return nullptr;

	auto &data = affFile->ReadAllBytes();
	Font_Imp* font = new Font_Imp(this,path,data);

	std::shared_ptr<FontReloadInformation> info;
	info.reset(new FontReloadInformation());
	info->ModifiedTime = GetResourceContainer()->GetModifiedTime(path);
	info->Path = path;

	GetResourceContainer()->Fonts.Regist(path, info, font);

	return font;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Chip2D* Graphics_Imp::CreateChip2D_()
{
	auto chip = new Chip2D_Imp(this);
	if (chip == nullptr) return nullptr;


	return chip;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Graphics_Imp::SetVertexBuffer(VertexBuffer_Imp* vertexBuffer)
{
	SafeAddRef(vertexBuffer);
	SafeRelease(m_vertexBufferPtr);
	m_vertexBufferPtr = vertexBuffer;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Graphics_Imp::SetIndexBuffer(IndexBuffer_Imp* indexBuffer)
{
	SafeAddRef(indexBuffer);
	SafeRelease(m_indexBufferPtr);
	m_indexBufferPtr = indexBuffer;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Graphics_Imp::SetShader(NativeShader_Imp* shader)
{
	SafeAddRef(shader);
	SafeRelease(m_shaderPtr);
	m_shaderPtr = shader;
}

void Graphics_Imp::SetRenderState(const RenderState& renderState)
{
	nextState.renderState = renderState;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Graphics_Imp::DrawPolygon(int32_t count)
{
	assert(m_vertexBufferPtr != nullptr);
	assert(m_indexBufferPtr != nullptr);
	assert(m_shaderPtr != nullptr);

	CommitRenderState(false);

	DrawPolygonInternal(
		count, 
		m_vertexBufferPtr,
		m_indexBufferPtr,
		m_shaderPtr);

	drawCallCountCurrent++;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Graphics_Imp::DrawPolygon(int32_t offset, int32_t count)
{
	assert(m_vertexBufferPtr != nullptr);
	assert(m_indexBufferPtr != nullptr);
	assert(m_shaderPtr != nullptr);

	CommitRenderState(false);

	DrawPolygonInternal(
		offset,
		count,
		m_vertexBufferPtr,
		m_indexBufferPtr,
		m_shaderPtr);

	drawCallCountCurrent++;
}

void Graphics_Imp::DrawPolygonInstanced(int32_t count, int32_t instanceCount)
{
	assert(m_vertexBufferPtr != nullptr);
	assert(m_indexBufferPtr != nullptr);
	assert(m_shaderPtr != nullptr);

	CommitRenderState(false);

	DrawPolygonInstancedInternal(
		count,
		m_vertexBufferPtr,
		m_indexBufferPtr,
		m_shaderPtr,
		instanceCount);

	drawCallCountCurrent++;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Graphics_Imp::Begin()
{
	drawCallCount = drawCallCountCurrent;
	drawCallCountCurrent = 0;

	CommitRenderState(true);

	ResetDrawState();

	BeginInternal();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Graphics_Imp::End()
{
	ResetDrawState();

	EndInternal();
}

void Graphics_Imp::Reload()
{
	Texture2DContainer->Reload([this](std::shared_ptr<ResourceContainer<Texture2D_Imp>::LoadingInformation> o, uint8_t* data, int32_t size) -> void
	{
		o->ResourcePtr->Reload(data, size);
	});

	EffectContainer->Reload([this](std::shared_ptr<ResourceContainer<Effect_Imp>::LoadingInformation> o, uint8_t* data, int32_t size) -> void
	{
		o->ResourcePtr->Reload(o->LoadedPath.c_str(), m_effectSetting, data, size);
	});

	GetResourceContainer()->Reload();

	for (auto& r : EffectContainer->GetAllResources())
	{
		auto e = r.second;
		e->ReloadResources(r.first.c_str());
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

}
