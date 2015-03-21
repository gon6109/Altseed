﻿#pragma once

#include <memory>
#include "ace.ProfilerViewer.h"
#include "../Window/ace.Window.h"
#include "../Graphics/2D/ace.Renderer2D_Imp.h"
#include "../Log/ace.Log.h"
#include "../Core/ace.Core.h"

namespace ace
{
	class ProfilerViewer_Imp : public ProfilerViewer
	{
	private:
		typedef struct TSprite
		{
			Vector2DF positions[4];
			Color colors[4];
			Vector2DF uvs[4];
		} Sprite;

		Profiler_Imp* m_profiler;
		Renderer2D* m_renderer;
		Graphics_Imp* m_graphics;
		Core* m_core;
		Vector2DI	m_windowSize;
		std::shared_ptr<Texture2D> m_materTexture;
		std::shared_ptr<Font> m_font;

		void AddBackgroundSprite(Renderer2D* renderer);
		void AddFpsSprite(Renderer2D* renderer, Core* core);
		void AddDrawCallSprite(Renderer2D* renderer, int drawCallCount);
		Sprite* CreatePolygonOfMater(int index, int time);
		void AddIdSprite(Renderer2D* renderer, int index, int id);
		void AddTimeSprite(Renderer2D* renderer, int index, int time);

		std::shared_ptr<Font> CreateFont_();

	public:
		ProfilerViewer_Imp(
			Graphics_Imp* graphics,
			Renderer2D* renderer,
			Log* log,
			Profiler_Imp* profiler,
			Core* core,
			Vector2DI windowSize);
		virtual ~ProfilerViewer_Imp();

		static ProfilerViewer_Imp* Create(
			Core* core,
			Profiler_Imp* profiler,
			Graphics_Imp* graphics,
			Log* logger,
			Vector2DI windowSize);

		void SetProfiler(Profiler_Imp* profiler);
		void Draw();
	};
}