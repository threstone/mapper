#include "mapper_util.h"
extern "C" _declspec(dllexport) const char* getTsPosition(const char* jsPath, int line, int column)
{
	return	MapperUtil::getTsPosition(jsPath, line, column);
}

extern "C" _declspec(dllexport) const char* getTsPositionByPtr(const char* jsPath, char* dataPtr, int line, int column)
{ 
	return	MapperUtil::getTsPositionByPtr(jsPath, dataPtr, line, column);
}