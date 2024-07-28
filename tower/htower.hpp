#pragma once
#include "../common/common.hpp"
#include "../common/framework.h"
#include "tower.h"
#include "../common/utils.hpp"
#include <iostream>

class item_aggregation_tower : public framework {
public:
	item_aggregation_tower(int mem, int hf_num, int max_win_num) : max_win_num(max_win_num), hf_num(hf_num) {
		tower = new TowerSketch*[max_win_num];
		for (int i = 0; i < max_win_num; ++i)
			tower[i] = nullptr;
		shk = new bool[max_win_num];
		for (int i = 0; i < max_win_num; ++i) shk[i] = false;
		for (int i = 1; (1 << i) <= max_win_num; ++i)
			shk[(1<<i) - 1] = true;
		double tot = 0; int cur = 1;
		for (int i = 0; i < max_win_num; ++i) {
			if (shk[i]) cur *= 2;
			tot += 1.0 / cur;
		}
		
		tower0_memory = mem / tot;
		tower0_memory = tower0_memory / cur / 8 * cur * 8;
		latest = 0;
	}

	~item_aggregation_tower() override {
		for (int i = 0; i < max_win_num; ++i)
			delete tower[i];
		delete[] tower;
		delete[] shk;
	}

	void new_window() {
		latest++;
		if (tower[max_win_num - 1] != nullptr)
			delete tower[max_win_num - 1];
		int cur = 2;
		for (int i = max_win_num - 1; i >= 1; --i) {
			tower[i] = tower[i-1];
			if (tower[i] && shk[i]) tower[i]->shrink();
		}
		tower[0] = new TowerSketch(tower0_memory, hf_num, max_win_num);
	}

	void add(int wid, elem_t e, int delta = 1) override {
		if (wid != latest) new_window();
		tower[0]->insert(e, wid, delta);
	}

	int query(int wid, elem_t e) const override {
		return tower[latest - wid]->query(e, wid);
	}

	int query_multiple_windows(int l, int r, elem_t e) const override {
		int sum = 0;
		for (int i = l; i <= r; ++i) sum += query(i, e);
		return sum;
	}

	int memory() const override {
		int mem_cnt = 0;
		// for (int i = 0; i < max_win_num; ++i)
		// 	if (tower[i]) mem_cnt += tower[i]->memory();
		return mem_cnt;
	}

	long long qcnt() const {
		long long sum = 0;
		for (int i = 0; i < max_win_num; ++i)
			if (tower[i]) sum += tower[i]->qcnt();
		return sum;
	}
	bool add_delta_implemented() const override { return false; }
	string name() const override { return "HTower"; }
	TowerSketch** tower;
private:
	
	int max_win_num, hf_num, tower0_memory;
	int latest;
	bool *shk;
};