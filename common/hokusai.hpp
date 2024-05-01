#pragma once
#include "common.hpp"
#include "../cmcubf/bloom_filter.hpp"
#include "../cmcubf/cm_sketch.hpp"
#include "../dasketch/da_sketch.hpp"
#include "../hyperloglog/hll_sketch.hpp"
#include "utils.hpp"
#include <iostream>

class item_aggregation_bf {
public:
	item_aggregation_bf(int mem, int hf_num, int max_win_num) : max_win_num(max_win_num), hf_num(hf_num) {
		bf = new bloom_filter*[max_win_num];
		for (int i = 0; i < max_win_num; ++i)
			bf[i] = nullptr;
		shk = new bool[max_win_num];
		for (int i = 0; i < max_win_num; ++i) shk[i] = false;
		for (int i = 1; (1 << i) <= max_win_num; ++i)
			shk[(1<<i) - 1] = true;
		double tot = 0; int cur = 1;
		for (int i = 0; i < max_win_num; ++i) {
			if (shk[i]) cur *= 2;
			tot += 1.0 / cur;
		}
		
		bf0_memory = mem / tot;
		bf0_memory = bf0_memory / cur / 8 * cur * 8;
		latest = 0;
	}

	void new_window() {
		latest++;
		if (bf[max_win_num - 1] != nullptr)
			delete bf[max_win_num - 1];
		int cur = 2;
		for (int i = max_win_num - 1; i >= 1; --i) {
			bf[i] = bf[i-1];
			if (bf[i] && shk[i]) bf[i]->shrink();
		}
		bf[0] = new bloom_filter(bf0_memory, hf_num);
	}

	void add(int wid, elem_t e) {
		if (wid != latest) new_window();
		bf[0]->add(e, wid);
	}

	void add(int wid, elem_t e, int t) { add(wid, e); }

	bool query(int wid, elem_t e) {
		return bf[latest - wid]->query(e, wid);
	}

	bool query_multiple_windows(int l, int r, elem_t e) {
		for (int i = l; i <= r; ++i)
			if (query(i, e)) return true;
		return false;
	}

	int memory() const {
		int mem_cnt = 0;
		for (int i = 0; i < max_win_num; ++i)
			if (bf[i]) mem_cnt += bf[i]->memory();
		return mem_cnt;
	}

	long long qcnt() const {
		long long sum = 0;
		for (int i = 0; i < max_win_num; ++i)
			if (bf[i]) sum += bf[i]->qcnt();
		return sum;
	}
	bool add_delta_implemented() const { return true; }
	bloom_filter** bf;
private:
	
	int max_win_num, hf_num, bf0_memory;
	int latest;
	bool *shk;
};

class time_aggregation_bf {
public:
	time_aggregation_bf(int mem, int hf_num, int lv_num) : hf_num(hf_num), lv_num(lv_num) {
		unit_mem = mem / (lv_num+1);
		bf = new bloom_filter*[lv_num];
		intv = new interval[lv_num];
		for (int i = 0; i < lv_num; ++i) {
			bf[i] = new bloom_filter(unit_mem, hf_num);
			if (i > 0) bf[i]->copy(bf[0]);
		}
		acm = new bloom_filter(unit_mem, hf_num);
		acm->copy(bf[0]);
		latest = 0;		
	}

	void new_window() {
		if (latest > 0) {
			bloom_filter *tmp = new bloom_filter(unit_mem, hf_num);
			for (int i = 0; latest % (1<<i) == 0; ++i) {
				tmp->copy(acm);
				acm->merge(bf[i]);
				bf[i]->copy(tmp);
				intv[i] = interval(latest - (1<<i) + 1, latest);
			}
			acm->clear(false);
			delete tmp;
		}
		latest++;
		
	}

	void add(int wid, elem_t e) {
		if (wid != latest) new_window();
		acm->add(e, wid);
	}

	void add(int wid, elem_t e, int t) { add(wid, e); }

	bool query(int wid, elem_t e) {
		if (wid == latest) return acm->query(e, wid);
		for (int i = 0; i < lv_num; ++i)
			if (intv[i].l <= wid && wid <= intv[i].r)
				return bf[i]->query(e, wid);
		return true;
	}

	bool query_multiple_windows(int l, int r, elem_t e) {
		for (int i = l; i <= r; ++i) if (query(i, e)) return true;
		return false;
	}

	int memory() const {
		int mem = acm->memory();
		for (int i = 0; i < lv_num; ++i) mem += bf[i]->memory();
		return mem;
	}

	long long qcnt() const {
		long long sum = acm->qcnt();
		for (int i = 0; i < lv_num; ++i) sum += bf[i]->qcnt();
		return sum;
	}
	bool add_delta_implemented() const { return true; }

	bloom_filter** bf, *acm;
private:	
	interval *intv;
	int lv_num, hf_num;
	int latest, unit_mem;
};

class item_aggregation_cm {
public:
	item_aggregation_cm(int mem, int hf_num, int max_win_num) : max_win_num(max_win_num), hf_num(hf_num) {
		cm = new cm_sketch*[max_win_num];
		for (int i = 0; i < max_win_num; ++i)
			cm[i] = nullptr;
		shk = new bool[max_win_num];
		for (int i = 0; i < max_win_num; ++i) shk[i] = false;
		for (int i = 1; (1 << i) <= max_win_num; ++i)
			shk[(1<<i) - 1] = true;
		double tot = 0; int cur = 1;
		for (int i = 0; i < max_win_num; ++i) {
			if (shk[i]) cur *= 2;
			tot += 1.0 / cur;
		}
		
		cm0_memory = mem / tot;
		cm0_memory = cm0_memory / cur / 8 * cur * 8;
		latest = 0;
	}

	void new_window() {
		latest++;
		if (cm[max_win_num - 1] != nullptr)
			delete cm[max_win_num - 1];
		int cur = 2;
		for (int i = max_win_num - 1; i >= 1; --i) {
			cm[i] = cm[i-1];
			if (cm[i] && shk[i]) cm[i]->shrink();
		}
		cm[0] = new cm_sketch(cm0_memory, hf_num);
	}

	void add(int wid, elem_t e, int delta = 1) {
		if (wid != latest) new_window();
		cm[0]->add(e, wid, delta);
	}

	int query(int wid, elem_t e) {
		return cm[latest - wid]->query(e, wid);
	}

	int query_multiple_windows(int l, int r, elem_t e) {
		int sum = 0;
		for (int i = l; i <= r; ++i) sum += query(i, e);
		return sum;
	}

	int memory() const {
		int mem_cnt = 0;
		for (int i = 0; i < max_win_num; ++i)
			if (cm[i]) mem_cnt += cm[i]->memory();
		return mem_cnt;
	}

	long long qcnt() const {
		long long sum = 0;
		for (int i = 0; i < max_win_num; ++i)
			if (cm[i]) sum += cm[i]->qcnt();
		return sum;
	}
	bool add_delta_implemented() const { return true; }
	cm_sketch** cm;
private:
	
	int max_win_num, hf_num, cm0_memory;
	int latest;
	bool *shk;
};

class item_aggregation_da : public framework {
public:
	item_aggregation_da(int mem, int hf_num, int max_win_num) : max_win_num(max_win_num), hf_num(hf_num) {
		da = new da_sketch*[max_win_num];
		for (int i = 0; i < max_win_num; ++i)
			da[i] = nullptr;
		shk = new bool[max_win_num];
		for (int i = 0; i < max_win_num; ++i) shk[i] = false;
		for (int i = 1; (1 << i) <= max_win_num; ++i)
			shk[(1<<i) - 1] = true;
		double tot = 0; int cur = 1;
		for (int i = 0; i < max_win_num; ++i) {
			if (shk[i]) cur *= 2;
			tot += 1.0 / cur;
		}
		
		da0_memory = mem / tot;
		da0_memory = da0_memory / cur / 8 * cur * 8;
		latest = 0;
	}

	~item_aggregation_da() override {
		for (int i = 0; i < max_win_num; ++i)
			delete da[i];
		delete[] da;
		delete[] shk;
	}

	void new_window() {
		latest++;
		if (da[max_win_num - 1] != nullptr)
			delete da[max_win_num - 1];
		int cur = 2;
		for (int i = max_win_num - 1; i >= 1; --i) {
			da[i] = da[i-1];
			if (da[i] && shk[i]) da[i]->shrink();
		}
		da[0] = new da_sketch(da0_memory, hf_num, 1);
	}

	void add(int wid, elem_t e, int delta = 1) override {
		if (wid != latest) new_window();
		da[0]->add(e, 0, delta);
	}

	int query(int wid, elem_t e) const override {
		return da[latest - wid]->query(e, wid);
	}

	pair<elem_t, int>** query_topk(pair<elem_t, int>** &result, int wid, int k = 1000) const override {
		pair<elem_t, int>** array_head = new pair<elem_t, int>*[2];
		result = array_head;
		*result = new pair<elem_t, int>[k];
		da[latest - wid]->query_topk(*result, k);
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
			if (da[i]) mem_cnt += da[i]->memory();
		return mem_cnt;
	}

	long long qcnt() const override {
		long long sum = 0;
		for (int i = 0; i < max_win_num; ++i)
			if (da[i]) sum += da[i]->qcnt();
		return sum;
	}
	bool add_delta_implemented() const override { return false; }
	da_sketch** da;
private:
	
	int max_win_num, hf_num, da0_memory;
	int latest;
	bool *shk;
};

class item_aggregation_hll : public framework {
public:
	item_aggregation_hll(int mem, int hf_num, int max_win_num) : max_win_num(max_win_num), hf_num(hf_num) {
		hll = new hll_sketch*[max_win_num];
		for (int i = 0; i < max_win_num; ++i)
			hll[i] = nullptr;
		shk = new bool[max_win_num];
		for (int i = 0; i < max_win_num; ++i) shk[i] = false;
		for (int i = 1; (1 << i) <= max_win_num; ++i)
			shk[(1<<i) - 1] = true;
		double tot = 0; int cur = 1;
		for (int i = 0; i < max_win_num; ++i) {
			if (shk[i]) cur *= 2;
			tot += 1.0 / cur;
		}
		
		hll0_memory = mem / tot;
		hll0_memory = hll0_memory / cur / 8 * cur * 8;
		latest = 0;
	}

	~item_aggregation_hll() override {
		for (int i = 0; i < max_win_num; ++i)
			delete hll[i];
		delete[] hll;
		delete[] shk;
	}

	void new_window() {
		latest++;
		if (hll[max_win_num - 1] != nullptr)
			delete hll[max_win_num - 1];
		int cur = 2;
		for (int i = max_win_num - 1; i >= 1; --i) {
			hll[i] = hll[i-1];
			if (hll[i] && shk[i]) hll[i]->shrink();
		}
		hll[0] = new hll_sketch(hll0_memory, hf_num, 1);
	}

	void add(int wid, elem_t e, int delta = 1) override {
		if (wid != latest) new_window();
		hll[0]->add(e, 0, delta);
	}

	int query(int wid) const override {
		return hll[latest - wid]->query();
	}

	int memory() const override {
		int mem_cnt = 0;
		for (int i = 0; i < max_win_num; ++i)
			if (hll[i]) mem_cnt += hll[i]->memory();
		return mem_cnt;
	}

	long long qcnt() const override {
		long long sum = 0;
		for (int i = 0; i < max_win_num; ++i)
			if (hll[i]) sum += hll[i]->qcnt();
		return sum;
	}
	bool add_delta_implemented() const override { return false; }
	hll_sketch** hll;
private:
	
	int max_win_num, hf_num, hll0_memory;
	int latest;
	bool *shk;
};