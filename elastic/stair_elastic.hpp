#pragma once
#include "../common/common.hpp"
#include "../common/framework.h"
#include "ElasticSketch.h"
#include <algorithm>
#include <iostream>
#include <climits>
#include <cassert>

class stair_level_elastic {
public:
	stair_level_elastic(int id, int mem, int elastic_num, int hf_num, int freq)
		: _id(id), elastic_num(elastic_num), refresh_freq(freq) {
		elastic = new ElasticSketch*[elastic_num];
		intv = new interval[elastic_num];

		for (int i = 0; i < elastic_num; ++i)
			elastic[i] = new ElasticSketch(mem / elastic_num);
		cur = 0;
		idx = 0;
		_cnt = 0;
	}

	~stair_level_elastic(){
		// printf("~stair_level_elastic()\n");
		for (int i = 0; i < elastic_num; ++i)
			delete elastic[i];
		delete[] elastic;
		delete[] intv;
	}

	void add_window() {
		if (cur % refresh_freq == 0 && cur > 0) {
			idx = (idx + 1) % elastic_num;
			intv[idx] = interval();
			elastic[idx]->clear();
		}
		++cur;
	}

	void add(elem_t e, int minv) {
		if (intv[idx].l == 0)  intv[idx].l = cur;
		if (intv[idx].r < cur) intv[idx].r = cur;
		elastic[idx]->insert(cur - 1, e, minv);
	}

	int query(int wid, elem_t e) const {
		for (int i = 0; i < elastic_num; ++i) 
			if (intv[i].l <= wid && wid <= intv[i].r)
				return elastic[i]->query(wid - 1, e);
		return INT_MAX;
	}
	
	void query_topk(pair<elem_t, int>** &result, int wid, int k = 1000) const {
		for (int i = 0; i < elastic_num; ++i) 
			if (intv[i].l <= wid && wid <= intv[i].r){
				elastic[i]->query_topk(result[0], wid - 1, k);
				result++;
			}
	}

	void notify(int l, int r) const {
		for (int i = 0; i < elastic_num; ++i)
			if (!(l > intv[i].r || r < intv[i].l))
				_cnt += elastic[i]->hfn();
	}

	int memory() const {
		int mem = 0;
		for (int i = 0; i < elastic_num; ++i) mem += elastic[i]->memory();
		return mem;
	}

	long long qcnt() const { return _cnt; }

private:
	ElasticSketch **elastic;
	const int refresh_freq;
	const int elastic_num;
	const int _id;
	mutable long long _cnt;
	
	int cur, idx;
	interval *intv;
};

class stair_elastic : public framework {
public:

	/*
		Absolute Memory Config Format
			(int memory, int ds_num, int hf_num, int refresh_freq)
	*/
	stair_elastic(const vector<tuple<int, int, int, int> > &level_config) 
		: lv_num(level_config.size()) {
		lv = new stair_level_elastic*[lv_num];
		for (int i = 0; i < lv_num; ++i) {
			const auto &config = level_config[i];
			lv[i] = new stair_level_elastic(i, get<0>(config), get<1>(config), 
				get<2>(config), get<3>(config));
		}
		_last_wid = -1;
	}

	~stair_elastic() override {
		printf("~stair_elastic()\n");
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

	pair<elem_t, int>** query_topk(pair<elem_t, int>** &result, int wid, int k = 1000) const override {
		pair<elem_t, int>** array_head = new pair<elem_t, int>*[2 << lv_num];
		result = array_head;
		for (int i = 0; i < lv_num; ++i)
			lv[i]->query_topk(result, wid, k);
		return array_head;
	}

	pair<elem_t, int>** query_multiple_windows_topk(pair<elem_t, int>** &result, int wid_start, int wid_end, int k = 1000) const override {
		for (int i = 0; i < lv_num; ++i)
			lv[i]->notify(wid_start, wid_end);
		return nullptr; 
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
	string name() const override { return "SElastic"; }

private:
	stair_level_elastic **lv;
	const int lv_num;
	int _last_wid;
};