#pragma once
#include "../common/common.hpp"
#include "hll_sketch.hpp"
#include <algorithm>
#include <iostream>
#include <climits>

class stair_level_hll {
public:
	stair_level_hll(int id, int mem, int hll_num, int hf_num, int freq)
		: _id(id), hll_num(hll_num), refresh_freq(freq) {
		hll = new hll_sketch*[hll_num];
		intv = new interval[hll_num];
		for (int i = 0; i < hll_num; ++i)
			hll[i] = new hll_sketch(mem / hll_num, hf_num);
		cur = 0; idx = 0;
	}

	void add_window() {
		if (cur % refresh_freq == 0 && cur != 0) {
			idx = (idx + 1) % hll_num;
			intv[idx] = interval();
			hll[idx]->clear();
		}
		++cur;
	}

	void add(elem_t e, int delta) {
		if (intv[idx].l == 0)  intv[idx].l = cur;
		if (intv[idx].r < cur) intv[idx].r = cur;
		hll[idx]->add(e, cur - 1, delta);
	}

	int query(int wid) const {
		for (int i = 0; i < hll_num; ++i) 
			if (intv[i].l <= wid && wid <= intv[i].r)
				return hll[i]->query();
		return INT_MAX;
	}

	void notify(int l, int r) const {
		for (int i = 0; i < hll_num; ++i)
			if (!(l > intv[i].r || r < intv[i].l))
				_cnt += 2 * hll[i]->hfn();
	}

	int memory() const {
		int mem = 0;
		for (int i = 0; i < hll_num; ++i) mem += hll[i]->memory();
		return mem;
	}

	long long qcnt() const { return _cnt; }

private:
	hll_sketch **hll;
	const int refresh_freq;
	const int hll_num;
	const int _id;
	
	int cur, idx;
	mutable long long _cnt;
	interval* intv;
};

class stair_hll : public framework {
public:

	/*
		Absolute Memory Config Format
			(int memory, int ds_num, int hf_num, int refresh_freq)
	*/
	stair_hll(const vector<tuple<int, int, int, int> > &level_config) 
		: lv_num(level_config.size()) {
		lv = new stair_level_hll*[lv_num];
		for (int i = 0; i < lv_num; ++i) {
			const auto &config = level_config[i];
			lv[i] = new stair_level_hll(i, get<0>(config), get<1>(config), 
				get<2>(config), get<3>(config));
		}
		_last_wid = -1;
	}

	~stair_hll() override {
		for (int i = 0; i < lv_num; ++i)
			delete lv[i];
		delete[] lv;
	}

	int query(int wid) const override {
		long long ret = 0;
		int valid_lv_number = 0;
		for (int i = 0; i < lv_num; ++i){
			int lv_result = lv[i]->query(wid);
			if (lv_result == INT_MAX) continue;
			valid_lv_number++;
			ret += lv_result;
		}
		return ret / valid_lv_number;
	}
	int query_multiple_windows(int wid_start, int wid_end, elem_t e) const {
		int sum = 0;
		for (int i = 0; i < lv_num; ++i)
			lv[i]->notify(wid_start, wid_end);
		// for (int wid = wid_start; wid <= wid_end; ++wid)
		// 	sum += query(wid);
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

	long long qcnt() const override {
		long long sum = 0;
		for (int i = 0; i < lv_num; ++i) sum += lv[i]->qcnt();
		return sum;
	}
	bool add_delta_implemented() const override { return false; }
	string name() const override { return "SHLL"; }

private:
	stair_level_hll **lv;
	const int lv_num;
	int _last_wid;
};