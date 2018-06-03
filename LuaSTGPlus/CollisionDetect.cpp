#include "CollisionDetect.h"

using namespace std;
using namespace LuaSTGPlus;

bool LuaSTGPlus::OBBHitTest(fcyVec2 P1, fcyVec2 Size1, float Angle1,
	fcyVec2 P2, fcyVec2 Size2, float Angle2)
{
	// ��������ε�4������
	fcyVec2 tFinalPos[2][4] =
	{
		{
			fcyVec2(-Size1.x, -Size1.y),
			fcyVec2(Size1.x, -Size1.y),
			fcyVec2(Size1.x, Size1.y),
			fcyVec2(-Size1.x, Size1.y)
		},
		{
			fcyVec2(-Size2.x, -Size2.y),
			fcyVec2(Size2.x, -Size2.y),
			fcyVec2(Size2.x, Size2.y),
			fcyVec2(-Size2.x, Size2.y)
		}
	};

	float tSin, tCos;
	{
		/*
		tSin = sin(Angle1);
		tCos = cos(Angle1);
		*/
		SinCos(Angle1, tSin, tCos);

		for (int i = 0; i<4; i++)
		{
			tFinalPos[0][i].RotationSC(tSin, tCos);
			tFinalPos[0][i] += P1;
		}
	}
	{
		/*
		tSin = sin(Angle2);
		tCos = cos(Angle2);
		*/
		SinCos(Angle2, tSin, tCos);

		for (int i = 0; i<4; i++)
		{
			tFinalPos[1][i].RotationSC(tSin, tCos);
			tFinalPos[1][i] += P2;
		}
	}

	// �����������ε�������
	for (int i = 0; i<2; i++)
	{
		fcyVec2 tAxis[2] =
		{
			tFinalPos[i][1] - tFinalPos[i][0],
			tFinalPos[i][2] - tFinalPos[i][1]
		};

		// ��λ������
		tAxis[0].Normalize();
		tAxis[1].Normalize();

		// ���ͶӰ�߶�
		fcyVec2 tAxisLine[2] =
		{
			fcyVec2(tFinalPos[i][0] * tAxis[0], tFinalPos[i][1] * tAxis[0]),
			fcyVec2(tFinalPos[i][1] * tAxis[1], tFinalPos[i][2] * tAxis[1])
		};

		// ��ÿһ������
		for (int j = 0; j<2; j++)
		{
			// ������һ���������ϵ�ͶӰ�������߶�
			fcyVec2 tProjLine(tFinalPos[1 - i][0] * tAxis[j], tFinalPos[1 - i][1] * tAxis[j]);
			if (tProjLine.y < tProjLine.x)
				std::swap(tProjLine.x, tProjLine.y);
			for (int k = 2; k<4; k++)
			{
				float v = tFinalPos[1 - i][k] * tAxis[j];
				if (v < tProjLine.x)
					tProjLine.x = v;
				if (v > tProjLine.y)
					tProjLine.y = v;
			}

			// ���и��ǲ���
			if (!OverlapTest(tAxisLine[j], tProjLine))
			{
				// �����ᶨ�ɣ�����һ�����ͶӰ���ཻ������ײ
				return false;
			}
		}
	}
	return true;
}

bool LuaSTGPlus::OBBCircleHitTest(fcyVec2 P1, fcyVec2 Size, float Angle,
	fcyVec2 P2, float R)
{
	// ��������ε�4������
	fcyVec2 tFinalPos[4] =
	{
		fcyVec2(-Size.x, -Size.y),
		fcyVec2(Size.x, -Size.y),
		fcyVec2(Size.x, Size.y),
		fcyVec2(-Size.x, Size.y)
	};

	// float tSin = sin(Angle), tCos = cos(Angle);
	float tSin, tCos;
	SinCos(Angle, tSin, tCos);

	// �任
	for (int i = 0; i<4; i++)
	{
		tFinalPos[i].RotationSC(tSin, tCos);
		tFinalPos[i] += P1;
	}

	// ������������
	fcyVec2 tAxis[2] =
	{
		tFinalPos[1] - tFinalPos[0],
		tFinalPos[2] - tFinalPos[1]
	};

	// �᳤��
	float tAxisLen[2] =
	{
		tAxis[0].Length(),
		tAxis[1].Length(),
	};

	// ��λ������
	if (tAxisLen[0] != 0.f)
		tAxis[0] /= tAxisLen[0];
	if (tAxisLen[1] != 0.f)
		tAxis[1] /= tAxisLen[1];

	// ����������Ĳο����������ϵ�ͶӰ
	float tProjValue[2] =
	{
		((tFinalPos[1] + tFinalPos[0]) / 2.f) * tAxis[0],
		((tFinalPos[2] + tFinalPos[1]) / 2.f) * tAxis[1]
	};

	// ����Բ�����ϵ�ͶӰ
	float tCircleCenterProjValue[2] =
	{
		P2 * tAxis[0],
		P2 * tAxis[1]
	};

	// ������ײ
	if (fabs(tCircleCenterProjValue[0] - tProjValue[0]) < tAxisLen[0] / 2.f)
		return (fabs(tCircleCenterProjValue[1] - tProjValue[1]) < tAxisLen[1] / 2.f + R);
	else if (fabs(tCircleCenterProjValue[1] - tProjValue[1]) < tAxisLen[1] / 2.f)
		return (fabs(tCircleCenterProjValue[0] - tProjValue[0]) < tAxisLen[0] / 2.f + R);

	// ����ĸ���
	float tDist2 = R;
	tDist2 *= tDist2;
	for (int i = 0; i < 4; i++)
	{
		if ((tFinalPos[i] - P2).Length2() < tDist2)
			return true;
	}

	return false;
}

bool LuaSTGPlus::OBBAABBHitTest(fcyVec2 P, fcyVec2 Size, float Angle, fcyRect Rect)
{
	// �����OBB���ε�4������
	fcyVec2 tFinalPos[2][4] =
	{
		{
			fcyVec2(-Size.x, -Size.y),
			fcyVec2(Size.x, -Size.y),
			fcyVec2(Size.x, Size.y),
			fcyVec2(-Size.x, Size.y)
		},
		{
			Rect.a,
			fcyVec2(Rect.b.x, Rect.a.y),
			Rect.b,
			fcyVec2(Rect.a.x, Rect.b.y)
		}
	};

	float tSin, tCos;
	{
		/*
		tSin = sin(Angle);
		tCos = cos(Angle);
		*/
		SinCos(Angle, tSin, tCos);

		for (int i = 0; i<4; i++)
		{
			tFinalPos[0][i].RotationSC(tSin, tCos);
			tFinalPos[0][i] += P;
		}
	}

	// �����������ε�������
	for (int i = 0; i<2; i++)
	{
		fcyVec2 tAxis[2] =
		{
			tFinalPos[i][1] - tFinalPos[i][0],
			tFinalPos[i][2] - tFinalPos[i][1]
		};

		// ��λ������
		tAxis[0].Normalize();
		tAxis[1].Normalize();

		// ���ͶӰ�߶�
		fcyVec2 tAxisLine[2] =
		{
			fcyVec2(tFinalPos[i][0] * tAxis[0], tFinalPos[i][1] * tAxis[0]),
			fcyVec2(tFinalPos[i][1] * tAxis[1], tFinalPos[i][2] * tAxis[1])
		};

		// ��ÿһ������
		for (int j = 0; j<2; j++)
		{
			// ������һ���������ϵ�ͶӰ�������߶�
			fcyVec2 tProjLine(tFinalPos[1 - i][0] * tAxis[j], tFinalPos[1 - i][1] * tAxis[j]);
			if (tProjLine.y < tProjLine.x)
				std::swap(tProjLine.x, tProjLine.y);
			for (int k = 2; k<4; k++)
			{
				float v = tFinalPos[1 - i][k] * tAxis[j];
				if (v < tProjLine.x)
					tProjLine.x = v;
				if (v > tProjLine.y)
					tProjLine.y = v;
			}

			// ���и��ǲ���
			if (!OverlapTest(tAxisLine[j], tProjLine))
			{
				// �����ᶨ�ɣ�����һ�����ͶӰ���ཻ������ײ
				return false;
			}
		}
	}
	return true;
}
