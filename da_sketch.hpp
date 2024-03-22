#pragma once
#include <cstring>
#include <climits>
#include <algorithm>
#include "hash.hpp"

class da_sketch {
public:	
	da_sketch(int memory, int hf_num) 
		: n(memory >> 2), hf_num(hf_num), hf(new hash_func[hf_num]) {
		pool = new int[n];
		memset(pool, 0, sizeof(int) * n);
		counter = 0;
	}

	void add(elem_t e, int offset, int minv) {
		for (int i = 0; i < hf_num; ++i) {
			int idx = (hf[i](e) + offset) % n;
			if (pool[idx] == minv) pool[idx]++;
		}
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