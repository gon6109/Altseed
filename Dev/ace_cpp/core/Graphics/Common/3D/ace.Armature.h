﻿
#pragma once

#include <Math/ace.Vector2DI.h>
#include <Math/ace.Vector2DF.h>
#include <Math/ace.Vector3DF.h>
#include <Math/ace.Matrix44.h>
#include <Graphics/ace.Color.h>

#include "../../../ace.Core.Base.h"
#include "../../../ace.ReferenceObject.h"

namespace ace
{
	class Armature
	{
	private:
		

	public:
		Armature() {}
		virtual ~Armature() {}

		virtual void AddBone(const achar* name, int32_t parentBoneIndex, eBoneRotationType rotationType, Matrix44 localMat, Matrix44 globalMatInv) = 0;
	};
}