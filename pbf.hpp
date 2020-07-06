#include "common.hpp"
#pragma once
#include "bloom_filter.hpp"
#include "utils.hpp"
#include <iostream>

class persistent_bf {
public:
	persistent_bf(int window_num, int mem_limit, int hf_num) : win_num(window_num) {
		depth = 0;
		check_depth(1, 1, win_num, 0);
		bf = new bloom_filter*[depth + 1];
		for (int i = 0; i <= depth; ++i) 
			bf[i] = new bloom_filter(mem_limit / (depth + 1), hf_num);
	}

	void check_depth(int x, int l, int r, int k) {
		depth = max(depth, k);
		if (l == r) return;
		int mid = (l + r) / 2;
		check_depth(x*2, l, mid, k+1);
		check_depth(x*2+1, mid+1, r, k+1);
	}

	bool query(int wid, elem_t e) const {
		return _query(1, 1, win_num, 0, wid, e);
	}

	bool query_multiple_windows(int wid_start, int wid_end, elem_t e) const {
		return _query(1, 1, win_num, 0, wid_start, wid_end, e);
	}

	void add(int wid, elem_t e) {
		_add(1, 1, win_num, 0, wid, e);
	}

	void add(int wid, elem_t e, int t) { add(wid, e); }

	int memory() const {
		int mem = 0;
		for (int i = 0; i <= depth; ++i) mem += bf[i]->memory();
		return mem;
	}

	long long qcnt() const {
		long long sum = 0;
		for (int i = 0; i <= depth; ++i) sum += bf[i]->qcnt();
		return sum;
	}

	bool add_delta_implemented() const { return true; }

private:
	bool _query(int x, int l, int r, int k, int p, elem_t e) const {
		if (!bf[k]->query(e, x)) return false;
		if (l == r) return true;
		int mid = (l + r) / 2;
		if (p <= mid) return _query(x * 2, l, mid, k + 1, p, e);
		else return _query(x * 2 + 1, mid + 1, r, k + 1, p, e);
	}

	bool _query(int x, int l, int r, int k, int L, int R, elem_t e) const {
		if (!bf[k]->query(e, x)) return false;
		if (L <= l && r <= R) return true;
		int mid = (l + r) / 2;
		if (L <= mid) {
			if (_query(x*2, l, mid, k+1, L, R, e)) return true;
		}
		if (mid < R) {
			if (_query(x*2+1, mid+1, r, k+1, L, R, e)) return true;
		}
		return false;
	}

	void _add(int x, int l, int r, int k, int p, elem_t e) {
		bf[k]->add(e, x);
		if (l == r) return;
		int mid = (l + r) / 2;
		if (p <= mid) _add(x * 2, l, mid, k + 1, p, e);
		else _add(x * 2 + 1, mid + 1, r, k + 1, p, e);
	}

	bloom_filter **bf;
	int depth, win_num;
};


class pbf1 {
public:
	pbf1(int window_num, int mem_limit, int hf_num) : win_num(window_num), hf_num(hf_num) {
		depth = 0;
		check_depth(1, 1, win_num, 0);
		unit_mem = mem_limit / (depth + 1);
		bf = new bloom_filter*[window_num * 4];
		build(1, 1, win_num, 0);
	}

	void check_depth(int x, int l, int r, int k) {
		depth = max(depth, k);
		if (l == r) return;
		int mid = (l + r) / 2;
		check_depth(x*2, l, mid, k+1);
		check_depth(x*2+1, mid+1, r, k+1);
	}

	void build(int x, int l, int r, int k) {
		bf[x] = new bloom_filter(unit_mem / (1<<k), hf_num);
		if (l == r) return;
		int mid = (l + r) / 2;
		build(x*2, l, mid, k+1);
		build(x*2+1, mid+1, r, k+1);
	}

	bool query(int wid, elem_t e) const {
		return _query(1, 1, win_num, 0, wid, e);
	}

	bool query_multiple_windows(int wid_start, int wid_end, elem_t e) const {
		return _query(1, 1, win_num, 0, wid_start, wid_end, e);
	}

	void add(int wid, elem_t e) { _add(1, 1, win_num, 0, wid, e); }

	void add(int wid, elem_t e, int t) { add(wid, e); }

	int memory() const {
		int mem = 0;
		for (int i = 0; i <= depth; ++i) mem += bf[i]->memory();
		return mem;
	}

	long long qcnt() const {
		long long sum = 0;
		for (int i = 0; i <= depth; ++i)
			sum += bf[i]->qcnt();
		return sum;
	}

	bool add_delta_implemented() const { return true; }

private:
	bool _query(int x, int l, int r, int k, int p, elem_t e) const {
		if (l == r) return bf[x]->query(e);
		int mid = (l + r) / 2;
		if (p <= mid) return _query(x * 2, l, mid, k + 1, p, e);
		else return _query(x * 2 + 1, mid + 1, r, k + 1, p, e);
	}

	bool _query(int x, int l, int r, int k, int L, int R, elem_t e) const {
		if (!bf[x]->query(e)) return false;
		if (L <= l && r <= R) return true;
		int mid = (l + r) / 2;
		if (L <= mid) {
			if (_query(x*2, l, mid, k+1, L, R, e)) return true;
		}
		if (mid < R) {
			if (_query(x*2+1, mid+1, r, k+1, L, R, e)) return true;
		}
		return false;
	}

	void _add(int x, int l, int r, int k, int p, elem_t e) {
		bf[x]->add(e);
		if (l == r) return;
		int mid = (l + r) / 2;
		if (p <= mid) _add(x * 2, l, mid, k + 1, p, e);
		else _add(x * 2 + 1, mid + 1, r, k + 1, p, e);
	}

	bloom_filter **bf;
	int depth, win_num, unit_mem, hf_num;
};

class pbf2 {
public:
	pbf2(int window_num, int mem_limit, int hf_num) : win_num(window_num) {
		depth = 0;
		check_depth(1, 1, win_num, 0);
		bf = new bloom_filter*[depth + 1];
		for (int i = 0; i <= depth; ++i) 
			bf[i] = new bloom_filter(mem_limit / (depth + 1), hf_num);
	}

	void check_depth(int x, int l, int r, int k) {
		depth = max(depth, k);
		if (l == r) return;
		int mid = (l + r) / 2;
		check_depth(x*2, l, mid, k+1);
		check_depth(x*2+1, mid+1, r, k+1);
	}

	bool query(int wid, elem_t e) const {
		return _query(1, 1, win_num, 0, wid, e);
	}

	bool query_multiple_windows(int wid_start, int wid_end, elem_t e) const {
		return _query(1, 1, win_num, 0, wid_start, wid_end, e);
	}

	void add(int wid, elem_t e) { _add(1, 1, win_num, 0, wid, e); }

	void add(int wid, elem_t e, int t) { add(wid, e); }

	int memory() const {
		int mem = 0;
		for (int i = 0; i <= depth; ++i) mem += bf[i]->memory();
		return mem;
	}

	long long qcnt() const {
		long long sum = 0;
		for (int i = 0; i <= depth; ++i)
			sum += bf[i]->qcnt();
		return sum;
	}

	bool add_delta_implemented() const { return true; }

private:
	bool _query(int x, int l, int r, int k, int p, elem_t e) const {
		if (l == r) return bf[k]->query(e, x);
		int mid = (l + r) / 2;
		if (p <= mid) return _query(x * 2, l, mid, k + 1, p, e);
		else return _query(x * 2 + 1, mid + 1, r, k + 1, p, e);
	}

	bool _query(int x, int l, int r, int k, int L, int R, elem_t e) const {
		if (!bf[k]->query(e, x)) return false;
		if (L <= l && r <= R) return true;
		int mid = (l + r) / 2;
		if (L <= mid) {
			if (_query(x*2, l, mid, k+1, L, R, e)) return true;
		}
		if (mid < R) {
			if (_query(x*2+1, mid+1, r, k+1, L, R, e)) return true;
		}
		return false;
	}

	void _add(int x, int l, int r, int k, int p, elem_t e) {
		bf[k]->add(e, x);
		if (l == r) return;
		int mid = (l + r) / 2;
		if (p <= mid) _add(x * 2, l, mid, k + 1, p, e);
		else _add(x * 2 + 1, mid + 1, r, k + 1, p, e);
	}

	bloom_filter **bf;
	int depth, win_num;
};