﻿#include "asd.CoreCircleShape_Imp.h"
#include "asd.CoreTriangleShape_Imp.h"
#include <Box2D/Box2D.h>

namespace asd
{
	CoreCircleShape_Imp::CoreCircleShape_Imp()
		:position(Vector2DF(0,0))
		, innerDiameter(0)
		, outerDiameter(0)
		, numberOfCorners(96)
		, angle(0)
	{

	}

	Vector2DF CoreCircleShape_Imp::GetPosition() const
	{
		return position;
	}

	void CoreCircleShape_Imp::SetPosition(Vector2DF position)
	{
		isNeededUpdating = true;
		isNeededCalcBoundingCircle = true;
		isNeededCalcCollisions = true;
		NotifyUpdateToObject();
		this->position = position;
	}

	float CoreCircleShape_Imp::GetAngle() const
	{
		return angle;
	}

	void CoreCircleShape_Imp::SetAngle(float angle)
	{
		isNeededUpdating = true;
		this->angle = angle;
	}

	float CoreCircleShape_Imp::GetOuterDiameter() const
	{
		return outerDiameter;
	}

	void CoreCircleShape_Imp::SetOuterDiameter(float outerDiameter)
	{
		isNeededUpdating = true;
		isNeededCalcBoundingCircle = true;
		isNeededCalcCollisions = true;
		NotifyUpdateToObject();
		this->outerDiameter = outerDiameter;
	}

	float CoreCircleShape_Imp::GetInnerDiameter() const
	{
		return innerDiameter;
	}

	void CoreCircleShape_Imp::SetInnerDiameter(float innerDiameter)
	{
		isNeededUpdating = true;
		isNeededCalcCollisions = true;
		this->innerDiameter = innerDiameter;
	}

	int CoreCircleShape_Imp::GetNumberOfCorners() const
	{
		return numberOfCorners;
	}

	void CoreCircleShape_Imp::SetNumberOfCorners(int numberOfCorners)
	{
		isNeededUpdating = true;
		this->numberOfCorners = numberOfCorners;
	}

	ShapeType CoreCircleShape_Imp::GetShapeType() const
	{
		return ShapeType::CircleShape;
	}

	ShapeType CoreCircleShape_Imp::GetType() const
	{
		return ShapeType::CircleShape;
	}

#if !SWIG
	void CoreCircleShape_Imp::DivideToTriangles()
	{
		if (numberOfCorners < 3) return;

		const float radInc = 360.0 / numberOfCorners;

		const float outerRadius = outerDiameter / 2;
		const float innerRadius = innerDiameter / 2;

		float currentPosDeg = angle - 90;
		float currentUVDeg = -90;

		Vector2DF baseVector(0, -1);
		baseVector.SetDegree(currentPosDeg);

		Vector2DF uvCenter = { 0.5, 0.5 };

		Vector2DF uvVector = { 0, -0.5 };

		float ratio = innerDiameter / outerDiameter;

		for (int i = 0; i < numberOfCorners; ++i)
		{
			Vector2DF currentPosVector = baseVector;
			currentPosVector.SetDegree(currentPosDeg);

			Vector2DF nextPosVector = baseVector;
			nextPosVector.SetDegree(currentPosDeg + radInc);

			Vector2DF currentUVVector = uvVector;
			currentUVVector.SetDegree(currentUVDeg);

			Vector2DF nextUVVector = uvVector;
			nextUVVector.SetDegree(currentUVDeg + radInc);

			std::array<Vector2DF, 4> vertexes = { position + currentPosVector*outerRadius, position + nextPosVector*outerRadius, position + nextPosVector*innerRadius, position + currentPosVector*innerRadius };
			std::array<Vector2DF, 4> uvs = { uvCenter + currentUVVector, uvCenter + nextUVVector, uvCenter + nextUVVector*ratio, uvCenter + currentUVVector*ratio };

			auto triangle1 = new CoreTriangleShape_Imp();

			for (int j = 0; j < 3; ++j)
			{
				triangle1->SetPointByIndex(vertexes[j], j);
				triangle1->SetUVByIndex(uvs[j], j);

			}

			triangles.push_back(triangle1);

			if (innerDiameter > 0)
			{
				auto triangle2 = new CoreTriangleShape_Imp();

				for (int j = 0; j < 3; ++j)
				{
					triangle2->SetPointByIndex(vertexes[(j + 2) % 4], j);
					triangle2->SetUVByIndex(uvs[(j + 2) % 4], j);
				}

				triangles.push_back(triangle2);
			}

			currentPosDeg += radInc;
			currentUVDeg += radInc;
		}
		
	}

	void CoreCircleShape_Imp::CalculateBoundingCircle()
	{
		boundingCircle = culling2d::Circle(culling2d::Vector2DF(position.X, position.Y), outerDiameter / 2.0f);
	}

	void CoreCircleShape_Imp::CalcCollisions()
	{
		if (collisionShapes.empty())
		{
			auto circle = new b2CircleShape();
			collisionShapes.push_back(circle);
		}

		auto circle = (b2CircleShape*)collisionShapes[0];


		circle->m_p = b2Vec2(position.X, position.Y);
		circle->m_radius = outerDiameter / 2;
	}
#endif

};