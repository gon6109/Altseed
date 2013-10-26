﻿#pragma once

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#include "../../Common/Resource/ace.IndexBuffer_Imp.h"
#include "../ace.Graphics_Imp_GL.h"

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace ace {
	//----------------------------------------------------------------------------------
	//
	//----------------------------------------------------------------------------------
	class IndexBuffer_Imp_GL
		: public IndexBuffer_Imp
	{
	private:
		GLuint			m_buffer;
		uint8_t*		m_lockedResource;

		IndexBuffer_Imp_GL(Graphics* graphics, GLuint buffer, int maxCount, bool isDynamic, bool is32bit);

	public:
		virtual ~IndexBuffer_Imp_GL();

		static IndexBuffer_Imp_GL* Create(Graphics* graphics, int maxCount, bool isDynamic, bool is32bit);

	public:
		void Lock();
		void Unlock();

		GLuint GetBuffer() { return m_buffer; }
	};

	//----------------------------------------------------------------------------------
	//
	//----------------------------------------------------------------------------------
}
