/**
 * $Id: ParameterSpaceHierarchy.cpp 152 2012-01-13 12:49:57Z ihulik $
 *
 * $Id: ParameterSpaceHierarchy.cpp 152 2012-01-13 12:49:57Z ihulik $
 * Developed by dcgm-robotics@FIT group
 * Author: Rostislav Hulik (ihulik@fit.vutbr.cz)
 * Date: dd.mm.2012 (version 1.0)
 *
 * License: BUT OPEN SOURCE LICENSE
 *
 * Description:
 *
 */

#include "plane_det/parameterSpaceHierarchy.h"

using namespace but_scenemodel;
using namespace std;
using namespace sensor_msgs;
using namespace cv;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ParameterSpaceHierarchy::ParameterSpaceHierarchy(double anglemin, double anglemax, double zmin, double zmax, double angleRes, double shiftRes)
{
	m_init = false;
	m_angleStep = (anglemax - anglemin) / (angleRes - 1);
	m_shiftStep = (zmax - zmin) / (shiftRes - 1);

	m_angleSize = (anglemax - anglemin) / m_angleStep + 1;
	m_angleSize2 = m_angleSize * m_angleSize;
	m_shiftSize = (zmax - zmin) / m_shiftStep + 1;

	assert(m_angleSize % DEFAULT_BIN_SIZE == 0 && m_shiftSize % DEFAULT_BIN_SIZE == 0);

	m_angleLoSize = m_angleSize / DEFAULT_BIN_SIZE;
	m_angleLoSize2 = m_angleSize2 / (DEFAULT_BIN_SIZE*DEFAULT_BIN_SIZE);
	m_shiftLoSize = m_shiftSize / DEFAULT_BIN_SIZE;

	m_shiftmin = zmin;
	m_shiftmax = zmax;
	m_anglemin = anglemin;
	m_anglemax = anglemax;
	m_size = m_angleSize2 * m_shiftSize;
	m_loSize = m_angleLoSize2 * m_shiftLoSize;

	m_hiSize = DEFAULT_BIN_SIZE*DEFAULT_BIN_SIZE*DEFAULT_BIN_SIZE;
	m_hiSize2 = DEFAULT_BIN_SIZE*DEFAULT_BIN_SIZE;
	m_dataLowRes = (double **)malloc(sizeof(double*)*m_loSize);
	for (int i = 0; i < m_loSize; ++i)
		m_dataLowRes[i] = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ParameterSpaceHierarchy::~ParameterSpaceHierarchy()
{
	for (int i = 0; i < m_loSize; ++i)
		if (m_dataLowRes[i] != NULL) free(m_dataLowRes[i]);

	free(m_dataLowRes);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ParameterSpaceHierarchy::toAngles(float x, float y, float z, float &a1, float &a2)
{
	a1 = atan2(z, x);
	// align with X on XZ
	x = z * sin(a1) + x * cos(a1);
	a2 = atan2(y, x);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ParameterSpaceHierarchy::toEuklid(float a1, float a2, float &x, float &y, float &z)
{
	float auxx = cos(-a2);
	y = -sin(-a2);
	x = auxx * cos(-a1);
	z = -auxx * sin(-a1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double ParameterSpaceHierarchy::get(int angle1, int angle2, int z)
{
	int loIndex = z/DEFAULT_BIN_SIZE * m_angleLoSize2 + angle2/DEFAULT_BIN_SIZE * m_angleLoSize + angle1/DEFAULT_BIN_SIZE;
	//std::cout << m_loSize << std::endl;
	//std::cout << loIndex << " -> " << z%DEFAULT_BIN_SIZE * m_hiSize2 + angle2%DEFAULT_BIN_SIZE * DEFAULT_BIN_SIZE + angle1%DEFAULT_BIN_SIZE << std::endl;
	//std::cout << std::endl;
	if (m_dataLowRes[loIndex] == NULL)
		return 0.0;
	else
		return m_dataLowRes[loIndex][z%DEFAULT_BIN_SIZE * m_hiSize2 + angle2%DEFAULT_BIN_SIZE * DEFAULT_BIN_SIZE + angle1%DEFAULT_BIN_SIZE];

}

void ParameterSpaceHierarchy::set(int angle1, int angle2, int z, double val)
{
	int loIndex = z/DEFAULT_BIN_SIZE * m_angleLoSize2 + angle2/DEFAULT_BIN_SIZE * m_angleLoSize + angle1/DEFAULT_BIN_SIZE;
	if (m_dataLowRes[loIndex] == NULL)
	{
		m_dataLowRes[loIndex] = (double *)malloc(sizeof(double) *m_hiSize);
		memset(m_dataLowRes[loIndex], 0, m_hiSize * sizeof(double));
	}
	m_dataLowRes[loIndex][z%DEFAULT_BIN_SIZE * m_hiSize2 + angle2%DEFAULT_BIN_SIZE * DEFAULT_BIN_SIZE + angle1%DEFAULT_BIN_SIZE] = val;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double ParameterSpaceHierarchy::get(int index)
{
	if (m_dataLowRes[index / m_hiSize] == NULL)
		return 0.0;
	else
		return m_dataLowRes[index / m_hiSize][index % m_hiSize];
}

void ParameterSpaceHierarchy::set(int index, double val)
{
	if (m_dataLowRes[index / m_hiSize] == NULL)
	{
		m_dataLowRes[index / m_hiSize] = (double *)malloc(sizeof(double) *m_hiSize);
		memset(m_dataLowRes[index / m_hiSize], 0, m_hiSize * sizeof(double));
	}
	m_dataLowRes[index / m_hiSize][index % m_hiSize] = val;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ParameterSpaceHierarchy::getIndex(double angle1, double angle2, double z)
{
	int a1 = (angle1-m_anglemin) / m_angleStep;
	int a2 = (angle2-m_anglemin) / m_angleStep;
	int zz = (z-m_shiftmin) / m_shiftStep;
	return zz * m_angleSize2 + a2 * m_angleSize + a1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ParameterSpaceHierarchy::getIndex(double angle1, double angle2, double z, int &angle1Index, int &angle2Index, int &shiftIndex)
{
	angle1Index = (angle1-m_anglemin) / m_angleStep;
	angle2Index = (angle2-m_anglemin) / m_angleStep;
    shiftIndex = (z-m_shiftmin) / m_shiftStep;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ParameterSpaceHierarchy::addVolume(ParameterSpace &second, int angle1, int angle2, int shift)
{
	double secondAngleHalf = second.m_angleSize / 2;
	double secondShiftHalf = second.m_shiftSize / 2;
	int shiftThis, angle1This, angle2This;
	int shiftS, angle1S, angle2S;

	for (shiftS = 0, shiftThis = shift - secondShiftHalf; shiftS < second.m_shiftSize; ++shiftS, ++shiftThis)
	for (angle2S=0, angle2This = angle2 - secondAngleHalf; angle2S < second.m_angleSize; ++angle2S, ++angle2This)
	for (angle1S=0, angle1This = angle1 - secondAngleHalf; angle1S < second.m_angleSize; ++angle1S, ++angle1This)
	{
		if (angle1This >= 0 && angle1This < m_angleSize &&
			angle2This >= 0 && angle2This < m_angleSize &&
			shiftThis  >= 0 && shiftThis < m_shiftSize)
		{
			set(angle1This, angle2This, shiftThis, get(angle1This, angle2This, shiftThis) + second(angle1S, angle2S, shiftS));
		}

	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//int ParameterSpaceHierarchy::findMaxima(std::vector<Plane<float> > &indices)
//{
//	int around = 2;
//	float a, b, c;
//	int maxind = -1;
//	float max = -1;
//
////	/////////////////////////////////////////////////
//	for (int shift=around; shift < m_shiftSize-around; ++shift)
//		{
//			for (int angle2=around; angle2 < m_angleSize-around; ++angle2)
//			{
//				for (int angle1=around; angle1 < m_angleSize-around; ++angle1)
//				{
//					double val = this->operator ()(angle1, angle2, shift);
//					if (val > 1500)
//					{
//						val += this->operator()(angle1-1, angle2, shift) +
//							   this->operator()(angle1+1, angle2, shift) +
//							   this->operator()(angle1, angle2-1, shift) +
//							   this->operator()(angle1, angle2+1, shift) +
//							   this->operator()(angle1, angle2, shift+1) +
//							   this->operator()(angle1, angle2, shift-1);
//
//						bool ok = true;
//						double aux;
//						int xx, yy, zz;
//						for (int x = -1; x <= 1; ++x)
//						for (int y = -1; y <= 1; ++y)
//						for (int z = -1; z <= 1; ++z)
//						{
//							xx = angle1 + x;
//							yy = angle2 + y;
//							zz = shift + z;
//							aux = this->operator()(xx, yy, zz) +
//								  this->operator()(xx-1, yy, zz) +
//								  this->operator()(xx+1, yy, zz) +
//								  this->operator()(xx, yy-1, zz) +
//								  this->operator()(xx, yy+1, zz) +
//								  this->operator()(xx, yy, zz+1) +
//								  this->operator()(xx, yy, zz-1);
//							if (val < aux)
//							{
//								ok = false;
//								break;
//							}
//						}
//
//						if (ok)
//						{
//							double aroundx = 0;
//							double aroundy = 0;
//							double aroundz = 0;
//							double arounds = 0;
//							for (int x = -1; x <= 1; ++x)
//							for (int y = -1; y <= 1; ++y)
//							for (int z = -1; z <= 1; ++z)
//							{
//								xx = angle1 + x;
//								yy = angle2 + y;
//								zz = shift + z;
//								toEuklid(getAngle(xx), getAngle(yy), a, b, c);
//								aroundx += a;
//								aroundy += b;
//								aroundz += c;
//								arounds += getShift(zz);
//							}
//							aroundx /= 7.0;
//							aroundy /= 7.0;
//							aroundz /= 7.0;
//							arounds /= 7.0;
//							std::cout << "Found plane size: " << val << " eq: " << aroundx <<" "<< aroundy <<" "<< aroundz <<" "<< arounds <<" "<< std::endl;
//							indices.push_back(Plane<float>(aroundx, aroundy, aroundz, arounds));
//							if (val > max)
//							{
//								max = val;
//								maxind = indices.size() - 1;
//							}
//						}
//					}
//				}
//			}
//		}
////	for (int shift=around; shift < m_shiftSize-around; ++shift)
////	{
////		for (int angle2=around; angle2 < m_angleSize-around; ++angle2)
////		{
////			for (int angle1=around; angle1 < m_angleSize-around; ++angle1)
////			{
////				double val = this->operator ()(angle1, angle2, shift);
////				if (val > 1500)
////				if (val > this->operator()(angle1-around, angle2, shift) &&
////					val > this->operator()(angle1+around, angle2, shift) &&
////					val > this->operator()(angle1, angle2-around, shift) &&
////					val > this->operator()(angle1, angle2+around, shift) &&
////					val > this->operator()(angle1, angle2, shift+around) &&
////					val > this->operator()(angle1, angle2, shift-around))
////				{
////
////					toEuklid(getAngle(angle1), getAngle(angle2), a, b, c);
////
////
////					indices.push_back(Plane<float>(a, b, c, getShift(shift)));
////					if (val > max)
////					{
////						max = val;
////						maxind = indices.size() - 1;
////					}
////					//std::cout << angle1 << " " << angle2 << " " << shift << std::endl;
////
////				}
////			}
////		}
////	}
//	//std::cout << "=========" << std::endl;
//	for(int i = 0; i < indices.size(); ++i)
//	{
//		int a1, a2, s;
//		toAngles(indices[i].a,indices[i].b, indices[i].c, a, b);
//		getIndex((double)a, (double)b, (double)indices[i].d, a1, a2, s);
//		this->operator()(a1-1, a2, s) = 0;
//		this->operator()(a1+1, a2, s) = 0;
//		this->operator()(a1, a2-1, s) = 0;
//		this->operator()(a1, a2+1, s) = 0;
//		this->operator()(a1, a2, s+1) = 0;
//		this->operator()(a1, a2, s-1) = 0;
//		//std::cout << a1 << " " << a2 << " " << s << std::endl;
//	}
//	return maxind;
//}
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double ParameterSpaceHierarchy::getAngle(int index)
{
	return (m_angleStep * ((double)index)) + m_anglemin;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double ParameterSpaceHierarchy::getShift(int index)
{
	return ((double)index) * m_shiftStep + m_shiftmin;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ParameterSpaceHierarchy::getSize()
{
	int size = 0;
	for (int i = 0; i < m_loSize; ++i)
		if (m_dataLowRes[i] != NULL) size += m_hiSize;
	return size;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double ParameterSpaceHierarchy::get(double angle1, double angle2, double z)
{
	int index = getIndex(angle1, angle2, z);

	if (m_dataLowRes[index / m_hiSize] == NULL)
			return 0.0;
		else
			return m_dataLowRes[index / m_hiSize][index % m_hiSize];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ParameterSpaceHierarchy::set(double angle1, double angle2, double z, double val)
{
	int index = getIndex(angle1, angle2, z);

	if (m_dataLowRes[index / m_hiSize] == NULL)
		{
			m_dataLowRes[index / m_hiSize] = (double *)malloc(sizeof(double) *m_hiSize);
			memset(m_dataLowRes[index / m_hiSize], 0, m_hiSize * sizeof(double));
		}
		m_dataLowRes[index / m_hiSize][index % m_hiSize] = val;
}


