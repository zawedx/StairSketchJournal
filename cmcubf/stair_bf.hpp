#pragma once
#include "../common/common.hpp"
#include "bloom_filter.hpp"
#include <algorithm>
#include <iostream>
#include <climits>

class stair_level_bf {
public:
	stair_level_bf(int id, int mem, int bf_num, int hf_num, int freq)
		: _id(id), bf_num(bf_num), refresh_freq(freq) {
		bf = new bloom_filter*[bf_num];
		intv = new interval[bf_num];
		for (int i = 0; i < bf_num; ++i)
			bf[i] = new bloom_filter(mem / bf_num, hf_num);

		cur = 0; idx = 0; _qcnt = 0;
	}

	void add_window() {
		if (cur % refresh_freq == 0 && cur > 0) {
			idx = (idx + 1) % bf_num;
			intv[idx] = interval();
			bf[idx]->clear();
		}
		++cur;
	}

	void add(elem_t e) {
		if (intv[idx].l == 0)  intv[idx].l = cur;
		if (intv[idx].r < cur) intv[idx].r = cur;
		bf[idx]->add(e, cur - 1);
	}

	bool query(int wid, elem_t e) const {
		for (int i = 0; i < bf_num; ++i) 
			if (intv[i].l <= wid && wid <= intv[i].r)
				return bf[i]->query(e, wid - 1);
		return true;
	}

	int query_mask(int l, int r, elem_t e) const {
		for (int i = 0; i < bf_num; ++i)
			if (!(l > intv[i].r || r < intv[i].l))
				_qcnt += bf[i]->hfn();
		int mask = 0;
		for (int i = l; i <= r; ++i)
			mask |= query(i, e) << i;
		return mask;
	}

	int memory() const {
		int mem = 0;
		for (int i = 0; i < bf_num; ++i) mem += bf[i]->memory();
		return mem;
	}

	long long qcnt() const {
		return _qcnt;
	}

private:
	bloom_filter **bf;
	const int refresh_freq;
	const int bf_num; // number of data structures
	const int _id;
	mutable long long _qcnt;
	
	int cur, idx;
	interval *intv;
};

class stair_bf {
public:
	stair_bf(const vector<tuple<int, int, int, int> > &level_config) 
		: lv_num(level_config.size()) {
		lv = new stair_level_bf*[lv_num];
		for (int i = 0; i < lv_num; ++i) {
			const auto &config = level_config[i];
			lv[i] = new stair_level_bf(i, get<0>(config), get<1>(config), get<2>(config), get<3>(config));
		}
		_last_wid = -1;
	}

	bool query(int wid, elem_t e) const {
		for (int i = 0; i < lv_num; ++i)
			if (!lv[i]->query(wid, e))
				return false;
		return true;
	}
	bool query_multiple_windows(int wid_start, int wid_end, elem_t e) const {
		int mask = INT_MAX;
		for (int i = 0; i < lv_num; ++i) {
			mask &= lv[i]->query_mask(wid_start, wid_end, e);
			if (mask == 0) return false;
		}
		return true;
	}
	void add(int wid, elem_t e) {
		if (wid != _last_wid) {
			_last_wid = wid;
			for (int i = 0; i < lv_num; ++i)
				lv[i]->add_window();
		}
		for (int i = 0; i < lv_num; ++i)
			lv[i]->add(e);
	}

	void add(int wid, elem_t e, int delta) { add(wid, e); }

	int memory() const {
		int mem = 0;
		for (int i = 0; i < lv_num; ++i) mem += lv[i]->memory();
		return mem;
	}

	long long qcnt() const {
		long long sum = 0;
		for (int i = 0; i < lv_num; ++i) sum += lv[i]->qcnt();
		return sum;
	}

	bool add_delta_implemented() const { return true; }

private:
	stair_level_bf **lv;
	const int lv_num;
	int _last_wid;
};