#define _CRT_SECURE_NO_DEPRECATE
#include "mapper_util.h"
#include <algorithm>
#include <string>

map<string, MapData*> cacheMap;
int useId = 0;

void split(string& str, vector<string>& v, string spacer);
char getRealNumByVLQ(char c);

MapData* MapperUtil::getMapInfo(const char* fileData) noexcept
{
	bool isBom = false;
	//判断是否是utf8-bom编码  如果是那么头三个字节分别是EF BB BF 对应-17 -69 -65
	if (fileData[0] == -17 && fileData[1] == -69 && fileData[2] == -65) {
		fileData += 3;
		isBom = true;
	}

	auto info = cJSON_Parse(fileData);
	if (info == NULL) {
		return NULL;
	}

	MapData* mapData = new MapData();

	mapData->version = cJSON_GetObjectItem(info, "version")->valueint;
	mapData->file = cJSON_GetObjectItem(info, "file")->valuestring;
	mapData->sourceRoot = cJSON_GetObjectItem(info, "sourceRoot")->valuestring;

	auto sources = cJSON_GetObjectItem(info, "sources");
	auto sourcesLength = cJSON_GetArraySize(sources);
	for (int i = 0; i < sourcesLength; i++) {
		string* tempStr = new string(cJSON_GetArrayItem(sources, i)->valuestring);
		mapData->sources.push_back(tempStr);
	}

	auto names = cJSON_GetObjectItem(info, "names");
	auto namesLength = cJSON_GetArraySize(names);
	for (int i = 0; i < namesLength; i++) {
		string* tempStr = new string(cJSON_GetArrayItem(names, i)->valuestring);
		mapData->names.push_back(tempStr);
	}

	mapData->mappings = cJSON_GetObjectItem(info, "mappings")->valuestring;

	delete info;
	if (isBom) {
		fileData -= 3;
	}
	return mapData;
}


char* MapperUtil::readFile(const char* name)
{
	FILE* file = fopen(name, "rb");
	if (file == NULL) return nullptr;

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	rewind(file);

	char* chars = new char[size + 1];
	chars[size] = '\0';
	for (size_t i = 0; i < size;) {
		i += fread(&chars[i], 1, size - i, file);
		if (ferror(file)) {
			fclose(file);
			return nullptr;
		}
	}
	fclose(file);
	return chars;
}

int MapperUtil::getNumByVLQ(string& vlqStr) {
	int len = vlqStr.length();
	int res = 0;

	//把连续位置为零
	for (int i = 0; i < len; i++) {
		//拿到需要的5个bit  把连续位置为零
		vlqStr[i] = vlqStr[i] & 0x1F;
	}

	//顺序颠倒过来
	reverse(vlqStr.begin(), vlqStr.end());

	//通过符号位获取符号
	bool isBigZero = (vlqStr[(long long)len - 1] | 1) != vlqStr[(long long)len - 1];
	//将最右符号位删除
	vlqStr[(long long)len - 1] = vlqStr[(long long)len - 1] >> 1;

	//合并最终值
	for (int i = 0; i < len; i++) {
		int temp = vlqStr[i];
		if (i != len - 1) {
			temp = temp << ((len - i - 1) * 5) - 1;
		}
		res = res | temp;
	}
	if (!isBigZero) {
		res = -res;
	}
	return res;
}


void  MapperUtil::getNumArrByVLQ(string vlqStr, vector<int>* v) {
	int curIndex = 0;
	int len = vlqStr.length();
	string tempStr = "";
	int pushTimes = 0;
	while (curIndex < len)
	{
		char temp = getRealNumByVLQ(vlqStr[curIndex]);
		tempStr += temp;
		//最后一位
		if ((temp | 32) != temp || curIndex + 1 == len) {
			v->push_back(getNumByVLQ(tempStr));
			tempStr = "";
			pushTimes++;
		}
		curIndex++;
	}
	if (pushTimes == 4) {
		v->push_back(0);
	}
}


void MapperUtil::initJsMap(const char* jsPath, MapData* data)
{
	//通过行列拿到对应mapping数据
	vector<string> lineArr;
	split(data->mappings, lineArr, ";");
	int lineLen = lineArr.size();

	//一行有多个元素,有多行
	int offsetArr[5] = { 0, 0, 0, 0, 0 };
	for (int i = 0; i < lineLen; i++) {
		vector<int>* lineVecPtr = new vector<int>();
		data->data.push_back(lineVecPtr);

		vector<string> columnArr;
		split(lineArr[i], columnArr, ",");
		for (int z = 0; z < columnArr.size(); z++) {
			getNumArrByVLQ(columnArr[z], lineVecPtr);
			(*lineVecPtr)[0 + z * 5] += offsetArr[0];
			(*lineVecPtr)[1 + (long long)z * 5] += offsetArr[1];
			(*lineVecPtr)[2 + (long long)z * 5] += offsetArr[2];
			(*lineVecPtr)[3 + (long long)z * 5] += offsetArr[3];
			(*lineVecPtr)[4 + (long long)z * 5] += offsetArr[4];
			offsetArr[0] = lineVecPtr->at(0 + z * 5);
			offsetArr[1] = lineVecPtr->at(1 + (long long)z * 5);
			offsetArr[2] = lineVecPtr->at(2 + (long long)z * 5);
			offsetArr[3] = lineVecPtr->at(3 + (long long)z * 5);
			offsetArr[4] = lineVecPtr->at(4 + (long long)z * 5);
		}
		offsetArr[0] = 0;
	}
	data->mappings = "";
	cacheMap[jsPath] = data;
	data->useId = useId++;

	//大于一百了,删除某个
	if (cacheMap.size() > 100) {
		string minPath = "";
		int minId = 0x7FFFFFFF;
		for (auto i = cacheMap.begin(); i != cacheMap.end(); i++) {
			if (i->second->useId < minId) {
				minId = i->second->useId;
				minPath = i->first;
			}
		}
		auto deleteData = cacheMap[minPath];
		int len = deleteData->data.size();
		for (int i = 0; i < len; i++) {
			delete deleteData->data[i];
		}

		len = deleteData->sources.size();
		for (int i = 0; i < len; i++) {
			delete deleteData->sources[i];
		}

		len = deleteData->names.size();
		for (int i = 0; i < len; i++) {
			delete deleteData->names[i];
		}

		delete deleteData;
		cacheMap.erase(minPath);
	}
}

const char* MapperUtil::getTsPosition(const char* jsPath, int line, int column)
{
	if (line < 1 || column < 1) {
		return "Error, The number of rows and columns must be greater than 0";//"行列数必须大于0"
	}

	if (cacheMap.find(jsPath) == cacheMap.end()) {
		//获取map数据
		string mapPath = jsPath;
		mapPath += ".map";

		auto fileData = readFile(mapPath.c_str());
		if (fileData == NULL) {
			return "Error, Do not find 'map' file";//没找到 .map文件
		}

		MapData* data = getMapInfo(fileData);
		delete[] fileData;
		if (data == NULL) {
			return "Error, Init MapData error";//初始化MapData出错了   可能是传入的json文件有问题
		}
		initJsMap(jsPath, data);
	}

	MapData* mapData = cacheMap[jsPath];
	return  mapData->getTsPosition(line, column);
}

const char* MapperUtil::getTsPositionByPtr(const char* jsPath, char* dataPtr, int line, int column)
{
	if (cacheMap.find(jsPath) == cacheMap.end()) {
		MapData* data = getMapInfo(dataPtr);
		if (data == NULL) {
			return  "getTsPositionByPtr,初始化MapData出错";
		}
		initJsMap(jsPath, data);
	}

	return getTsPosition(jsPath, line, column);
}

void split(string& str, vector<string>& v, string spacer)
{
	int pos1, pos2;
	int len = spacer.length();     //记录分隔符的长度
	pos1 = 0;
	pos2 = str.find(spacer);
	while (pos2 != string::npos)
	{// 2 4 8 16 32
		v.push_back(str.substr(pos1, pos2 - (long long)pos1));
		pos1 = pos2 + len;
		pos2 = str.find(spacer, pos1);    // 从str的pos1位置开始搜寻spacer
	}
	if (pos1 != str.length()) //分割最后一个部分
		v.push_back(str.substr(pos1));
}

char getRealNumByVLQ(char c)
{
	int temp = 0;
	//A~Z
	if (c >= 65 && c <= 90) {
		temp = c - 65;
	}
	else if (c >= 97 && c <= 122) {
		temp = c - 71;
	}
	else if (c >= 48 && c <= 57) {
		temp = c + 4;
	}
	else if (c == 43) {
		temp = 62;
	}
	else if (c == 47) {
		temp = 63;
	}
	return temp;
}



const char* MapData::getTsPosition(int line, int column)
{
	char res[512] = { 0 };
	useId = useId++;
	if (line > data.size()) {
		return "Error, The line number entered exceeds the maximum range";// 输入的行号超出了最大范围
	}
	auto lineData = data[(long long)line - 1];
	int len = lineData->size();
	for (long long i = 0; i < len; i += 5) {
		int curColumn = lineData->at(i);
		if (curColumn >= column) {
			break;
		}
		if (curColumn == column - 1) {
			string resStr = *(sources[lineData->at(i + 1)]) + ":" + to_string(lineData->at(i + 2) + 1);
			resStr += ":" + to_string(lineData->at(i + 3) + 1);
			
			auto resChar = resStr.c_str();
			int resSize = strlen(resChar);
			resSize = resSize > 511 ? 511 : resSize;
			for (int z = 0; z < resSize; z++) {
				res[z] = resChar[z];
			}
			res[resSize] = '\0';
			return res;
		}
	}
	return NULL;
}
