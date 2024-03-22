#pragma once
#include "common.hpp"
#include "da_sketch.hpp"
#include <algorithm>
#include <iostream>
#include <climits>
#include <cassert>

class stair_level_da {
public:
	stair_level_da(int id, int mem, int da_num, int hf_num, int freq)
		: _id(id), da_num(da_num), refresh_freq(freq) {
		da = new da_sketch*[da_num];
		intv = new interval[da_num];

		for (int i = 0; i < da_num; ++i)
			da[i] = new da_sketch(mem / da_num, hf_num);
		cur = 0;
		idx = 0;
		_cnt = 0;
	}

	void add_window() {
		if (cur % refresh_freq == 0 && cur > 0) {
			idx = (idx + 1) % da_num;
			intv[idx] = interval();
			da[idx]->clear();
		}
		++cur;
	}

	void add(elem_t e, int minv) {
		if (intv[idx].l == 0)  intv[idx].l = cur;
		if (intv[idx].r < cur) intv[idx].r = cur;
		da[idx]->add(e, cur - 1, minv);
	}
	
	int query(int wid, elem_t e) const {
		for (int i = 0; i < da_num; ++i) 
			if (intv[i].l <= wid && wid <= intv[i].r)
				return da[i]->query(e, wid - 1);
		return INT_MAX;
	}

	void notify(int l, int r) const {
		for (int i = 0; i < da_num; ++i)
			if (!(l > intv[i].r || r < intv[i].l))
				_cnt += 2 * da[i]->hfn();
	}

	int memory() const {
		int mem = 0;
		for (int i = 0; i < da_num; ++i) mem += da[i]->memory();
		return mem;
	}

	long long qcnt() const { return _cnt; }

private:
	da_sketch **da;
	const int refresh_freq;
	const int da_num;
	const int _id;
	mutable long long _cnt;
	
	int cur, idx;
	interval *intv;
};

class stair_da {
public:

	/*
		Absolute Memory Config Format
			(int memory, int ds_num, int hf_num, int refresh_freq)
	*/
	stair_da(const vector<tuple<int, int, int, int> > &level_config) 
		: lv_num(level_config.size()) {
		lv = new stair_level_da*[lv_num];
		for (int i = 0; i < lv_num; ++i) {
			const auto &config = level_config[i];
			lv[i] = new stair_level_da(i, get<0>(config), get<1>(config), 
				get<2>(config), get<3>(config));
		}
		_last_wid = -1;
	}

	int query(int wid, elem_t e) const {
		int ret = INT_MAX;
		for (int i = 0; i < lv_num; ++i)
			ret = min(ret, lv[i]->query(wid, e));
		return ret;
	}
	
	int query_multiple_windows(int wid_start, int wid_end, elem_t e) const {
		int sum = 0;
		for (int i = 0; i < lv_num; ++i)
			lv[i]->notify(wid_start, wid_end);
		for (int wid = wid_start; wid <= wid_end; ++wid)
			sum += query(wid, e);
		return sum;
	}

	void add(int wid, elem_t e) {
		if (wid != _last_wid) {
			_last_wid = wid;
			for (int i = 0; i < lv_num; ++i)
				lv[i]->add_window();
		}
		int minv = query(wid, e);
		for (int i = 0; i < lv_num; ++i)
			lv[i]->add(e, minv);
	}

	void add(int wid, elem_t e, int delta) {
		assert(false); // not implemented
	}

	int memory() const {
		int mem = 0;
		for (int i = 0; i < lv_num; ++i) mem += lv[i]->memory();
		return mem;
	}

	int qcnt() const {
		int sum = 0;
		for (int i = 0; i < lv_num; ++i) sum += lv[i]->qcnt();
		return sum;
	}

	bool add_delta_implemented() const { return false; }

private:
	stair_level_da **lv;
	const int lv_num;
	int _last_wid;
};