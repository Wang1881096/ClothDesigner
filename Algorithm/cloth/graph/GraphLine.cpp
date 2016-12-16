#include "GraphLine.h"
#include "tinyxml\tinyxml.h"
#include "GraphPoint.h"
namespace ldp
{
	GraphLine::GraphLine() : AbstractGraphCurve()
	{
		m_keyPoints.resize(2, nullptr);
	}
	GraphLine::GraphLine(size_t id) : AbstractGraphCurve(id)
	{
		m_keyPoints.resize(2, nullptr);
	}
	GraphLine::GraphLine(const std::vector<GraphPoint*>& pts, size_t id) : AbstractGraphCurve(pts, id)
	{
		assert(pts.size() == 2);
	}

	float GraphLine::calcLength()const
	{
		for (int k = 0; k < numKeyPoints(); k++)
		{
			assert(m_keyPoints[k]);
		}
		return (m_keyPoints[1]->position() - m_keyPoints[0]->position()).length(); 
	}

	Float2 GraphLine::getPointByParam(float t)const
	{
		for (int k = 0; k < numKeyPoints(); k++)
		{
			assert(m_keyPoints[k]);
		}
		return m_keyPoints[0]->position() * (1 - t) + m_keyPoints[1]->position() * t;
	}
}