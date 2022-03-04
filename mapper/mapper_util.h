#pragma once
#include "cJSON.h"
#include <iostream>
#include <vector>
#include <map>
using namespace std;


struct MapData
{
	int version;
	string file;
	string sourceRoot;
	vector<string*> sources;
	vector<string*>  names;
	string mappings;
	vector<vector<int>*> data;
	int useId;
	const char* getTsPosition(int line, int column);
};

class MapperUtil
{
private:
	static MapData* getMapInfo(const char* fileData) noexcept;
	static char* readFile(const char* name);
	static void getNumArrByVLQ(string vlqStr, vector<int>* v);
	static void initJsMap(const char* jsPath, MapData* data);
	static int getNumByVLQ(string& vlqStr);
public:
	 static const char* getTsPosition(const char* jsPath, int line, int column);
	static const char * getTsPositionByPtr(const char* jsPath, char* dataPtr, int line, int column);
};

