// Chinese Postman problem
#include <iostream>
#include <string>
#include "../include/chinese_postman.h"

int main(int argc, char** argv){
	ChinesePostman cp;
	bool ok = false;
	if (argc >= 2) {
		ok = cp.loadFromFile(argv[1]);
		if (!ok) { std::cerr << "无法打开文件: " << argv[1] << "\n"; return 1; }
	} else {
		ok = cp.loadFromStdin();
		if (!ok) { std::cerr << "从标准输入读取失败。\n"; return 1; }
	}
	int start = 0;
	std::cout << "请输入邮局起点编号(1-based,默认1)然后回车：";
	std::string s; if (std::getline(std::cin, s)) {
		if (!s.empty()) try { start = std::stoi(s)-1; } catch(...) { start = 0; }
	}
	auto res = cp.solve(start);
	if (res.total_cost < 0) {
		std::cout << "图不连通或无法求解中国邮路问题。\n";
		return 1;
	}
	std::cout << "最短总路程: " << res.total_cost << "\n";
	std::cout << "投递路线顶点序列(1-based):\n";
	for (size_t i=0;i<res.euler_path.size();++i) {
		std::cout << (res.euler_path[i]+1) << (i+1==res.euler_path.size()?"\n":" -> ");
	}
	return 0;
}

