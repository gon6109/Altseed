﻿
#include <list>
#include "ace.CoreLayer2D_Imp.h"
#include "../../Window/ace.Window_Imp.h"
#include "../../Graphics/Common/ace.Graphics_Imp.h"
#include "../../Graphics/Common/2D/ace.LayerRenderer.h"

using namespace std;

namespace ace
{
	//----------------------------------------------------------------------------------
	//
	//----------------------------------------------------------------------------------
	CoreLayer2D_Imp::CoreLayer2D_Imp(Graphics* graphics, Log* log, Vector2DI windowSize)
		: CoreLayer_Imp(graphics, windowSize)
		, m_objects(list<ObjectPtr>())
		, m_renderer(nullptr)
	{
		m_renderer = new Renderer2D_Imp(graphics, log, windowSize);
	}

	//----------------------------------------------------------------------------------
	//
	//----------------------------------------------------------------------------------
	CoreLayer2D_Imp::~CoreLayer2D_Imp()
	{
		SafeDelete(m_renderer);

		for (auto& object : m_objects)
		{
			SafeRelease(object);
		}

	}

	//----------------------------------------------------------------------------------
	//
	//----------------------------------------------------------------------------------
	void CoreLayer2D_Imp::AddObject(ObjectPtr object)
	{
		m_objects.push_back(object);
		object->SetLayer(this);
		SafeAddRef(object);
	}

	//----------------------------------------------------------------------------------
	//
	//----------------------------------------------------------------------------------
	void CoreLayer2D_Imp::RemoveObject(ObjectPtr object)
	{
		m_objects.remove(object);
		object->SetLayer(nullptr);
		SafeRelease(object);
	}

	void CoreLayer2D_Imp::BeginUpdating()
	{

	}

	void CoreLayer2D_Imp::EndUpdating()
	{

	}

	//----------------------------------------------------------------------------------
	//
	//----------------------------------------------------------------------------------
	void CoreLayer2D_Imp::BeginDrawing()
	{
		m_targetToLayer = -1;
		m_triangles.clear();
	}

	//----------------------------------------------------------------------------------
	//
	//----------------------------------------------------------------------------------
	void CoreLayer2D_Imp::Draw()
	{
		if (!m_isDrawn)
		{
			return;
		}

		for (auto& x : m_objects)
		{
			x->Draw();
		}
	}

	//----------------------------------------------------------------------------------
	//
	//----------------------------------------------------------------------------------
	void CoreLayer2D_Imp::EndDrawing()
	{
		if (m_postEffects.size() > 0)
		{
			m_graphics->SetRenderTarget(m_renderTarget0, nullptr);
		}

		m_renderer->DrawCache();
		m_renderer->ClearCache();
	}

	Renderer2D* CoreLayer2D_Imp::GetRenderer() const
	{
		return m_renderer;
	}

	RenderTexture2D* CoreLayer2D_Imp::GetFirstRenderTarget()
	{
		return m_renderTarget0;
	}
}