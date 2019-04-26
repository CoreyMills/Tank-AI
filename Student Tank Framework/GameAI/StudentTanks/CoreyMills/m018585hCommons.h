#ifndef M018585HCOMMON_H
#define M018585HCOMMON_H

#include "../../Commons.h"
#include <vector>

class GameObject;
#include "../../GameObject.h"

class BaseTank;
#include "../../BaseTank.h"

inline double Cross(const Vector2D& v1, const Vector2D& v2)
{
	return v1.x * v2.y - v2.x * v1.y;
}

//returns false if raycast hits an object
inline bool Raycast(Vector2D startPos, Vector2D endPos, vector<GameObject*> obstacles)
{
	Vector2D startToEnd = endPos - startPos;
	int loopAmount = (int)((startToEnd.Length()) / kTileDimensions) * 8;
	Vector2D sectionSize = startToEnd / loopAmount;

	//can leftCheck see rightCheck
	for (int i = 1; i <= loopAmount; i++)
	{
		Vector2D checkPos = startPos + (sectionSize * i);
		for (GameObject* obstacle : obstacles)
		{
			vector<Vector2D> objBox = obstacle->GetAdjustedBoundingBox();
			if (InsideRegion(checkPos, objBox.at(0), objBox.at(2)))
				return false;
		}
	}
	return true;
}

//return true if p1 is closer to target than p2
inline bool ClosestPoint(Vector2D p1, Vector2D p2, Vector2D target)
{
	return (Vec2DDistanceSq(p1, target) < Vec2DDistanceSq(p2, target));
}
#endif // !M018585HCOMMON_H