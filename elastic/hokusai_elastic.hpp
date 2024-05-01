#pragma once
#include "../common/common.hpp"
#include "../common/framework.h"
#include "../common/utils.hpp"
#include "ElasticSketch.h"
#include <iostream>

class item_aggregation_elastic : public framework {
public:
	item_aggregation_elastic(int mem, int hf_num, int max_win_num) : max_win_num(max_win_num), hf_num(hf_num) {
		elastic = new ElasticSketch*[max_win_num];
		for (int i = 0; i < max_win_num; ++i)
			elastic[i] = nullptr;
		shk = new bool[max_win_num];
		for (int i = 0; i < max_win_num; ++i) shk[i] = false;
		for (int i = 1; (1 << i) <= max_win_num; ++i)
			shk[(1<<i) - 1] = true;
		double tot = 0; int cur = 1;
		for (int i = 0; i < max_win_num; ++i) {
			if (shk[i]) cur *= 2;
			tot += 1.0 / cur;
		}
		
		elastic0_memory = mem / tot;
		elastic0_memory = elastic0_memory / cur / 8 * cur * 8;
		latest = 0;
	}

	~item_aggregation_elastic() override {
		for (int i = 0; i < max_win_num; ++i)
			delete elastic[i];
		delete[] elastic;
		delete[] shk;
	}

	void new_window() {
		latest++;
		if (elastic[max_win_num - 1] != nullptr)
			delete elastic[max_win_num - 1];
		int cur = 2;
		for (int i = max_win_num - 1; i >= 1; --i) {
			elastic[i] = elastic[i-1];
			if (elastic[i] && shk[i]) elastic[i]->shrink();
		}
		elastic[0] = new ElasticSketch(elastic0_memory, 1);
	}

	void add(int wid, elem_t e, int delta = 1) override {
		if (wid != latest) new_window();
		elastic[0]->insert(wid, e, delta);
	}

	int query(int wid, elem_t e) const override {
		return elastic[latest - wid]->query(e, wid);
	}

	pair<elem_t, int>** query_topk(pair<elem_t, int>** &result, int wid, int k = 1000) const override {
		pair<elem_t, int>** array_head = new pair<elem_t, int>*[2];
		result = array_head;
		*result = new pair<elem_t, int>[k];
		elastic[latest - wid]->query_topk(*result, wid, k);
		++result;
		return array_head;
	}

	int query_multiple_windows(int l, int r, elem_t e) const override {
		int sum = 0;
		for (int i = l; i <= r; ++i) sum += query(i, e);
		return sum;
	}

	int memory() const override {
		int mem_cnt = 0;
		for (int i = 0; i < max_win_num; ++i)
			if (elastic[i]) mem_cnt += elastic[i]->memory();
		return mem_cnt;
	}

	long long qcnt() const override {
		int sum = 0;
		for (int i = 0; i < max_win_num; ++i)
			if (elastic[i]) sum += elastic[i]->qcnt();
		return sum;
	}
	bool add_delta_implemented() const override { return false; }
	ElasticSketch** elastic;
private:
	
	int max_win_num, hf_num, elastic0_memory;
	int latest;
	bool *shk;
};