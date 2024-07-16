#pragma once
#include "../common/common.hpp"
#include "cu_sketch.hpp"
#include "cu_sketch.hpp"
#include <algorithm>
#include <iostream>
#include <climits>
#include <cassert>

class stair_level_cu {
public:
	stair_level_cu(int id, int mem, int cu_num, int hf_num, int freq)
		: _id(id), cu_num(cu_num), refresh_freq(freq) {
		cu = new cu_sketch*[cu_num];
		intv = new interval[cu_num];

		for (int i = 0; i < cu_num; ++i)
			cu[i] = new cu_sketch(mem / cu_num, hf_num);
		cur = 0;
		idx = 0;
		_cnt = 0;
	}

	~stair_level_cu(){
		for (int i = 0; i < cu_num; ++i)
			delete cu[i];
		delete[] intv;
		delete[] cu;
	}

	void add_window() {
		if (cur % refresh_freq == 0 && cur > 0) {
			idx = (idx + 1) % cu_num;
			intv[idx] = interval();
			cu[idx]->clear();
		}
		++cur;
	}

	void add(elem_t e, int minv) {
		if (intv[idx].l == 0)  intv[idx].l = cur;
		if (intv[idx].r < cur) intv[idx].r = cur;
		cu[idx]->add(e, cur - 1, minv);
	}
	
	int query(int wid, elem_t e) const {
		for (int i = 0; i < cu_num; ++i) 
			if (intv[i].l <= wid && wid <= intv[i].r)
				return cu[i]->query(e, wid - 1);
		return INT_MAX;
	}

	void notify(int l, int r) const {
		for (int i = 0; i < cu_num; ++i)
			if (!(l > intv[i].r || r < intv[i].l))
				_cnt += 2 * cu[i]->hfn();
	}

	int memory() const {
		int mem = 0;
		for (int i = 0; i < cu_num; ++i) mem += cu[i]->memory();
		return mem;
	}

	long long qcnt() const { return _cnt; }

private:
	cu_sketch **cu;
	const int refresh_freq;
	const int cu_num;
	const int _id;
	mutable long long _cnt;
	
	int cur, idx;
	interval *intv;
};

class stair_cu : public framework {
public:

	/*
		Absolute Memory Config Format
			(int memory, int ds_num, int hf_num, int refresh_freq)
	*/
	stair_cu(const vector<tuple<int, int, int, int> > &level_config) 
		: lv_num(level_config.size()) {
		lv = new stair_level_cu*[lv_num];
		for (int i = 0; i < lv_num; ++i) {
			const auto &config = level_config[i];
			lv[i] = new stair_level_cu(i, get<0>(config), get<1>(config), 
				get<2>(config), get<3>(config));
		}
		_last_wid = -1;
	}

	~stair_cu() override {
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
		for (int i = 0; i < lv_num; ++i)
			lv[i]->notify(wid_start, wid_end);
		for (int wid = wid_start; wid <= wid_end; ++wid)
			sum += query(wid, e);
		return sum;
	}

	void add(int wid, elem_t e, int delta = 1) override {
		if (wid != _last_wid) {
			_last_wid = wid;
			for (int i = 0; i < lv_num; ++i)
				lv[i]->add_window();
		}
		int minv = query(wid, e);
		for (int i = 0; i < lv_num; ++i)
			lv[i]->add(e, minv);
	}

	int memory() const override {
		int mem = 0;
		for (int i = 0; i < lv_num; ++i) mem += lv[i]->memory();
		return mem;
	}

	long long qcnt() const override {
		int sum = 0;
		for (int i = 0; i < lv_num; ++i) sum += lv[i]->qcnt();
		return sum;
	}

	bool add_delta_implemented() const { return false; }
	string name() const override { return "SCU"; }

private:
	stair_level_cu **lv;
	const int lv_num;
	int _last_wid;
};