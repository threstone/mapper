// mapper.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>
#include "mapper_util.h"

using namespace std;

int main(int argc, char* argv[])
{
	cout << MapperUtil::getTsPosition("E:\\mapper\\mapper\\test\\dist\\main.js", 2, 11);
	return 0;

	//有3个参数才调用
	if (argc >= 4) {
		auto path = argv[1];//E:\dacc\server\dist\dacc_player.js     mapper.exe E:\dacc\server\dist\main.js 12 11
		auto line = std::atoi(argv[2]);
		auto column = std::atoi(argv[3]);
		//cout << "MapperUtil::getTsPosition(" << path << "," << line << "," << column << ");\n";
		cout << MapperUtil::getTsPosition(path, line, column);
		//MapperUtil::getTsPosition("E:\\dacc\\server\\dist\\dacc_player.js", 12, 1);
	}
	else {
		cout << "参数不足..." << endl;
	}
	return 0;
}