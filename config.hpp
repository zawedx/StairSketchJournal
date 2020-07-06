#pragma once

#include <string>

class config {
public:
	config() {}

	config(std::string dataset, int win_num, int ds_win_num, double win_time, int memory) 
		: dataset(dataset), win_num(win_num), ds_win_num(ds_win_num), win_time(win_time), memory(memory) {}

	std::string dataset;
	int win_num, ds_win_num;
	double win_time;
	int memory;
};

config cfg;