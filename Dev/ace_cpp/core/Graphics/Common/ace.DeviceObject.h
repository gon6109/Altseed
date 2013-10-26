﻿#pragma once

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#include <ace.common.Base.h>
#include "../../ace.Core.Base.h"
#include "../../ace.ReferenceObject.h"

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace ace {
	//----------------------------------------------------------------------------------
	//
	//----------------------------------------------------------------------------------
	class DeviceObject
		: public ReferenceObject
	{
	private:
		Graphics*	m_graphics;

	public:
		DeviceObject(Graphics* graphics);
		virtual ~DeviceObject();

		Graphics* GetGraphics() const { return m_graphics; }
		virtual void OnLostDevice() {}
		virtual void OnResetDevice() {}
	};

	//----------------------------------------------------------------------------------
	//
	//----------------------------------------------------------------------------------

}