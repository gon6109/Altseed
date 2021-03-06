﻿#include <cmath>
#include "asd.CoreShape_Imp.h"
#include "asd.CoreTriangleShape_Imp.h"
#include "asd.CoreLineShape_Imp.h"
#include "asd.CoreRectangleShape_Imp.h"
#include "asd.CoreCircleShape_Imp.h"
#include "asd.CorePolygonShape_Imp.h"
#include "asd.CoreArcShape_Imp.h"
#include <Box2D/Box2D.h>
#include "asd.CoreShapeConverter.h"
#include "../ObjectSystem/2D/asd.CoreGeometryObject2D_Imp.h"

#if defined(_WIN32)

#ifdef _WIN64

#if _DEBUG
#pragma comment(lib,"x64/Debug/Box2D.lib")
#else
#pragma comment(lib,"x64/Release/Box2D.lib")
#endif

#else

#if _DEBUG
#pragma comment(lib,"x86/Debug/Box2D.lib")
#else
#pragma comment(lib,"x86/Release/Box2D.lib")
#endif

#endif

#endif

namespace asd
{

	CoreShape_Imp::~CoreShape_Imp()
	{
		for (auto triangle : triangles)
		{
			SafeRelease(triangle);
		}

		for (auto col : collisionShapes)
		{
			delete col;
		}
	}

	bool CoreShape_Imp::GetIsCollidedb2Shapes(CoreShape* shape)
	{
		auto shape_Imp = CoreShape2DToImp(shape);
		for (auto selfShape : GetCollisionShapes())
		{
			for (auto theirsShape : shape_Imp->GetCollisionShapes())
			{

				b2Transform identity = b2Transform();
				identity.SetIdentity();

				bool isOverlap = b2TestOverlap(selfShape, 1, theirsShape, 1, identity, identity);


				if (isOverlap)
				{
					return true;
				}
			}
		}
		return false;
	}
#if !SWIG
	void CoreShape_Imp::SetGeometryObject2D(CoreGeometryObject2D_Imp* geometryObject)
	{
		this->geometryObject = geometryObject;
	}


	culling2d::Circle& CoreShape_Imp::GetBoundingCircle()
	{
		if (isNeededCalcBoundingCircle)
		{
			CalculateBoundingCircle();
			isNeededCalcBoundingCircle = false;
		}
		return boundingCircle;
	}

	void CoreShape_Imp::NotifyUpdateToObject()
	{
		if (geometryObject != nullptr)
		{
			geometryObject->SetIsUpdateBoundingCircle();
		}
	}


	bool CoreShape_Imp::GetIsCollidedWithCircleAndRect(CoreCircleShape* circle, CoreRectangleShape* rectangle)
	{
		auto untransformedVertexes = rectangle->GetDrawingArea().GetVertexes();

		Vector2DF untransformedOrigin = (untransformedVertexes[0] + untransformedVertexes[1] + untransformedVertexes[2] + untransformedVertexes[3]) / 4;

		Vector2DF vec = (untransformedOrigin - (untransformedVertexes[0] + rectangle->GetCenterPosition()));
		vec.SetDegree(vec.GetDegree() + rectangle->GetAngle());

		Vector2DF rectGlobalCenter = untransformedVertexes[0] + rectangle->GetCenterPosition() + vec;

		Vector2DF c;

		c.X = std::cos(rectangle->GetAngle()) * (circle->GetPosition().X - rectGlobalCenter.X) -
			std::sin(rectangle->GetAngle()) * (circle->GetPosition().Y - rectGlobalCenter.Y) + rectGlobalCenter.X;
		c.Y = std::sin(rectangle->GetAngle()) * (circle->GetPosition().X - rectGlobalCenter.X) +
			std::cos(rectangle->GetAngle()) * (circle->GetPosition().Y - rectGlobalCenter.Y) + rectGlobalCenter.Y;

		Vector2DF nearestPos;

		Vector2DF rectGlobalPos = rectangle->GetDrawingArea().GetPosition();
		Vector2DF rectSize = rectangle->GetDrawingArea().GetSize();

		if (c.X < rectGlobalPos.X)
		{
			nearestPos.X = rectGlobalPos.X;
		}
		else if (c.X > rectGlobalPos.X + rectSize.X)
		{
			nearestPos.X = rectGlobalPos.X + rectSize.X;
		}
		else
		{
			nearestPos.X = c.X;
		}

		if (c.Y < rectGlobalPos.Y)
		{
			nearestPos.Y = rectGlobalPos.Y;
		}
		else if (c.Y > rectGlobalPos.Y + rectSize.Y)
		{
			nearestPos.Y = rectGlobalPos.Y + rectSize.Y;
		}
		else
		{
			nearestPos.Y = c.Y;
		}

		float radius2 = pow(circle->GetOuterDiameter() / 2, 2);

		float dist = (c - nearestPos).GetSquaredLength();

		return dist < radius2;
	}

	bool CoreShape_Imp::GetIsCollidedWithCircleAndLine(CoreCircleShape* circle, CoreLineShape* line)
	{
		std::shared_ptr<CoreRectangleShape> rect = std::make_shared<CoreRectangleShape_Imp>();

		Vector2DF gravPos = (line->GetStartingPosition() + line->GetEndingPosition()) / 2;
		float halfLen = (line->GetStartingPosition() - gravPos).GetLength();

		rect->SetDrawingArea(RectF(-halfLen + gravPos.X, -line->GetThickness() / 2 + gravPos.Y, halfLen * 2, line->GetThickness()));

		return GetIsCollidedWithCircleAndRect(circle, rect.get());
	}

	std::vector<b2Shape*>& CoreShape_Imp::GetCollisionShapes()
	{
		if (isNeededCalcCollisions)
		{
			if (GetType() == ShapeType::PolygonShape || GetType() == ShapeType::ArcShape)
			{
				for (auto shape : collisionShapes)
				{
					delete shape;
				}
				collisionShapes.clear();
			}
			CalcCollisions();
			isNeededCalcCollisions = false;
		}
		return collisionShapes;
	}

#endif
};