#pragma once
#include "Global.h"
#include <fcyMath.h>

namespace LuaSTGPlus
{
	inline void SinCos(float ang, float& fSin, float& fCos)
	{
		float s, c;

		__asm fld ang
		__asm fsincos
		__asm fstp c
		__asm fstp s

		fSin = s;
		fCos = c;
	}

	/// @brief �߶θ��ǲ���
	/// @note  ��Ҫ��֤lineA.x < lineA.y �� lineB.x < lineB.y
	inline bool OverlapTest(fcyVec2 lineA, fcyVec2 lineB)
	{
		if (lineA.x < lineB.x)
			return lineA.y > lineB.x;
		else
			return lineB.y > lineA.x;
	}

	/// @brief Բ���ཻ����
	inline bool CircleHitTest(fcyVec2 P1, float R1, fcyVec2 P2, float R2)
	{
		fcyVec2 tOffset = P1 - P2;
		float tRTotal = R1 + R2;

		return tOffset.Length2() < tRTotal * tRTotal;
	}

	/// @brief �������(AABB)�ཻ����
	/// @param[in] P1    ����1������λ��
	/// @param[in] Size1 ����1�İ�߳�
	/// @param[in] P2    ����2������λ��
	/// @param[in] Size2 ����2�İ�߳�
	inline bool AABBHitTest(fcyVec2 P1, fcyVec2 Size1, fcyVec2 P2, fcyVec2 Size2)
	{
		fcyRect tRect1(P1.x - Size1.x, P1.y - Size1.y, P1.x + Size1.x, P1.y + Size1.y);
		fcyRect tRect2(P2.x - Size2.x, P2.y - Size2.y, P2.x + Size2.x, P2.y + Size2.y);

		return tRect1.Intersect(tRect2, NULL);
	}

	/// @brief OBB���������ײ���
	/// @param[in] P1     ����1����
	/// @param[in] Size1  ����1��߳�
	/// @param[in] Angle1 ����1��ת
	/// @param[in] P2     ����2����
	/// @param[in] Size2  ����2��߳�
	/// @param[in] Angle2 ����2��ת
	bool OBBHitTest(
		fcyVec2 P1, fcyVec2 Size1, float Angle1,
		fcyVec2 P2, fcyVec2 Size2, float Angle2);

	/// @brief OBB���������Բ��ײ���
	/// @param[in] P1    ��������
	/// @param[in] Size  ���ΰ�߳�
	/// @param[in] Angle ������ת
	/// @param[in] P2    Բ����
	/// @param[in] R     Բ�뾶
	bool OBBCircleHitTest(
		fcyVec2 P1, fcyVec2 Size, float Angle,
		fcyVec2 P2, float R);

	/// @brief OBB���������AABB��Χ����ײ���
	/// @param[in] P     ��������
	/// @param[in] Size  ���ΰ�߳�
	/// @param[in] Angle ������ת
	/// @param[in] Rect  AABBλ��
	bool OBBAABBHitTest(
		fcyVec2 P, fcyVec2 Size, float Angle,
		fcyRect Rect);
}
