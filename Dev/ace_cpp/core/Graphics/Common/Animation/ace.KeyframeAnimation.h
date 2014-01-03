﻿
#pragma once

#include "../../../ace.Core.Base.h"
#include "../../../ace.ReferenceObject.h"

#include <Math/ace.Vector2DF.h>

namespace ace
{
	class FCurveKeyframe
	{
	public:
		Vector2DF				KeyValue;
		Vector2DF				LeftHandle;
		Vector2DF				RightHandle;
		eInterpolationType		InterpolationType;

		FCurveKeyframe()
		{
			InterpolationType = eInterpolationType::INTERPOLATION_TYPE_LINEAR;
		}
	};

	class KeyframeAnimation
		: public IReference
	{
	public:
		KeyframeAnimation() {}
		virtual ~KeyframeAnimation() {}

		virtual const achar* GetName() = 0;

		virtual void SetName(const achar* name) = 0;

		virtual void AddKeyframe(const FCurveKeyframe& kf) = 0;

		virtual float GetValue(float frame) = 0;
	};
}