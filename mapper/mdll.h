#ifndef MDLL_H_
#define MDLL_H_
extern "C" _declspec(dllexport)const char* getTsPosition(const char* jsPath, int line, int column);
extern "C" _declspec(dllexport)const char* getTsPositionByPtr(const char* jsPath, char* dataPtr, int line, int column);
#endif