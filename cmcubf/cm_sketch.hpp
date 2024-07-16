#pragma once
#include <cstring>
#include <climits>
#include <algorithm>
#include "../common/hash.hpp"

class cm_sketch {
public:	
	cm_sketch(int memory, int hf_num) 
		: n(memory >> 2), hf_num(hf_num), hf(new hash_func[hf_num]) {
		pool = new int[n];
		memset(pool, 0, sizeof(int) * n);
		counter = 0;
	}

	~cm_sketch(){
		delete[] pool;
	}

	void add(elem_t e, int offset = 0, int delta = 1) {
		for (int i = 0; i < hf_num; ++i) 
			pool[(hf[i](e) + offset) % n] += delta;
		++counter;
	}

	int query(elem_t e, int offset = 0) const {
		_cnt += hf_num;
		int res = INT_MAX;
		for (int i = 0; i < hf_num; ++i)
			res = std::min(res, pool[(hf[i](e) + offset) % n]);
		return res;
	}

	void shrink() {
	#ifdef DEBUG
		assert(_size % 2 == 0);
	#endif
		n /= 2;
		int *new_pool = new int[n];
		for (int i = 0; i < n; ++i)
			new_pool[i] = pool[i] + pool[i + n];
		delete[] pool;
		pool = new_pool;
	}

	void clear() {
		for (int i = 0; i < hf_num; ++i)
			hf[i].reset(); // Use new hash functions
		counter = 0;
		memset(pool, 0, sizeof(int) * n);
	}

	int size() const { return n; }
	int memory() const { return n * 4; }

	long long qcnt() const { return _cnt; }
	double usage() const { return 1.0 * counter / n; }
	int hfn() const { return hf_num; }

private:
	mutable long long _cnt;
	int n, hf_num;
	int *pool;
	int counter;
	hash_func* hf;
};