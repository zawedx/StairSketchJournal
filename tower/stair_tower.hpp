#pragma once
#include "../common/common.hpp"
#include "tower.h"
#include <algorithm>
#include <iostream>
#include <climits>

class stair_level_tower {
public:
	stair_level_tower(int id, int mem, int tower_num, int hf_num, int freq)
		: _id(id), tower_num(tower_num), refresh_freq(freq) {
		tower = new TowerSketch*[tower_num];
		intv = new interval[tower_num];
		for (int i = 0; i < tower_num; ++i)
			tower[i] = new TowerSketch(mem / tower_num, hf_num);
		cur = 0; idx = 0;
	}

	void add_window() {
		if (cur % refresh_freq == 0 && cur != 0) {
			idx = (idx + 1) % tower_num;
			intv[idx] = interval();
			tower[idx]->clear();
		}
		++cur;
	}

	void add(elem_t e, int delta) {
		if (intv[idx].l == 0)  intv[idx].l = cur;
		if (intv[idx].r < cur) intv[idx].r = cur;
		tower[idx]->insert(cur - 1, e, delta);
	}

	int query(int wid, elem_t e) const {
		for (int i = 0; i < tower_num; ++i) 
			if (intv[i].l <= wid && wid <= intv[i].r)
				return tower[i]->query(wid - 1, e);
		return INT_MAX;
	}

	int memory() const {
		int mem = 0;
		// for (int i = 0; i < tower_num; ++i) mem += tower[i]->memory();todo
		return mem;
	}

private:
	TowerSketch **tower;
	const int refresh_freq;
	const int tower_num;
	const int _id;
	
	int cur, idx;
	interval* intv;
};

class stair_tower : public framework {
public:

	/*
		Absolute Memory Config Format
			(int memory, int ds_num, int hf_num, int refresh_freq)
	*/
	stair_tower(const vector<tuple<int, int, int, int> > &level_config) 
		: lv_num(level_config.size()) {
		lv = new stair_level_tower*[lv_num];
		for (int i = 0; i < lv_num; ++i) {
			const auto &config = level_config[i];
			lv[i] = new stair_level_tower(i, get<0>(config), get<1>(config), 
				get<2>(config), get<3>(config));
		}
		_last_wid = -1;
	}

	~stair_tower() override {
		for (int i = 0; i < lv_num; ++i)
			delete lv[i];
		delete[] lv;
	}

	int query(int wid, elem_t e) const override {
		int ret = INT_MAX;
		for (int i = 0; i < lv_num; ++i)
			ret = min(ret, lv[i]->query(wid, e));
		return ret;
	}
	int query_multiple_windows(int wid_start, int wid_end, elem_t e) const override {
		int sum = 0;
		for (int wid = wid_start; wid <= wid_end; ++wid)
			sum += query(wid, e);
		return sum;
	}
	void add(int wid, elem_t e, int delta = 1) override {
		if (wid != _last_wid) {
			_last_wid = wid;
			for (int i = 0; i < lv_num; ++i) lv[i]->add_window();
		}
		for (int i = 0; i < lv_num; ++i) lv[i]->add(e, delta);
	}
	int memory() const override {
		int mem = 0;
		for (int i = 0; i < lv_num; ++i) mem += lv[i]->memory();
		return mem;
	}
	bool add_delta_implemented() const override { return false; }

private:
	stair_level_tower **lv;
	const int lv_num;
	int _last_wid;
};