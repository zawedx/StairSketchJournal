#pragma once
#include "../common/common.hpp"
#include "cm_sketch.hpp"
#include <algorithm>
#include <iostream>
#include <climits>

class stair_level_cm {
public:
	stair_level_cm(int id, int mem, int cm_num, int hf_num, int freq)
		: _id(id), cm_num(cm_num), refresh_freq(freq) {
		cm = new cm_sketch*[cm_num];
		intv = new interval[cm_num];
		for (int i = 0; i < cm_num; ++i)
			cm[i] = new cm_sketch(mem / cm_num, hf_num);
		cur = 0; idx = 0;
	}

	~stair_level_cm(){
		for (int i = 0; i < cm_num; ++i)
			delete cm[i];
		delete[] intv;
		delete[] cm;
	}

	void add_window() {
		if (cur % refresh_freq == 0 && cur != 0) {
			idx = (idx + 1) % cm_num;
			intv[idx] = interval();
			cm[idx]->clear();
		}
		++cur;
	}

	void add(elem_t e, int delta) {
		if (intv[idx].l == 0)  intv[idx].l = cur;
		if (intv[idx].r < cur) intv[idx].r = cur;
		cm[idx]->add(e, cur - 1, delta);
	}

	int query(int wid, elem_t e) const {
		for (int i = 0; i < cm_num; ++i) 
			if (intv[i].l <= wid && wid <= intv[i].r)
				return cm[i]->query(e, wid - 1);
		return INT_MAX;
	}

	void notify(int l, int r) const {
		for (int i = 0; i < cm_num; ++i)
			if (!(l > intv[i].r || r < intv[i].l))
				_cnt += 2 * cm[i]->hfn();
	}

	int memory() const {
		int mem = 0;
		for (int i = 0; i < cm_num; ++i) mem += cm[i]->memory();
		return mem;
	}

	long long qcnt() const { return _cnt; }

private:
	cm_sketch **cm;
	const int refresh_freq;
	const int cm_num;
	const int _id;
	
	int cur, idx;
	mutable long long _cnt;
	interval* intv;
};

class stair_cm : public framework {
public:

	/*
		Absolute Memory Config Format
			(int memory, int ds_num, int hf_num, int refresh_freq)
	*/
	stair_cm(const vector<tuple<int, int, int, int> > &level_config) 
		: lv_num(level_config.size()) {
		lv = new stair_level_cm*[lv_num];
		for (int i = 0; i < lv_num; ++i) {
			const auto &config = level_config[i];
			lv[i] = new stair_level_cm(i, get<0>(config), get<1>(config), 
				get<2>(config), get<3>(config));
		}
		_last_wid = -1;
	}

	~stair_cm() override {
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
		int sum = 0;
		for (int i = 0; i < lv_num; ++i) sum += lv[i]->qcnt();
		return sum;
	}
	bool add_delta_implemented() const override { return true; }
	string name() const override { return "SCM"; }

private:
	stair_level_cm **lv;
	const int lv_num;
	int _last_wid;
};