#pragma once
#include <cstring>
#include <climits>
#include <algorithm>
#include "../common/hash.hpp"

class csm_sketch {
public:	
	csm_sketch(int memory, int hf_num, int shrink_flag = 0) 
		: n(memory >> 2), hf_num(hf_num), hf(new hash_func[hf_num]) {
		if (shrink_flag) n = n / shrink_flag * shrink_flag;
		pool = new int[n];
		memset(pool, 0, sizeof(int) * n);
		counter = 0;
	}

	~csm_sketch(){
		delete[] pool;
	}

	void add(elem_t e, int offset = 0, int delta = 1) {
		for (int i = 0; i < hf_num; ++i) 
			pool[(hf[i](e) + offset) % n] += delta;
        counter += delta;
	}

	int query(elem_t e, int offset = 0, int opt = 1) const {
		_cnt += hf_num;
		int res = INT_MAX;
		for (int i = 0; i < hf_num; ++i)
			res = std::min(res, pool[(hf[i](e) + offset) % n]);
		if (opt == 0)
			return res;
		else if (opt == 1)
			return res - hf_num * (counter - res) / (n - 1);
		else assert(false);
	}
// todo shrink max or plus? 
	void shrink() {
		assert(n % 2 == 0);
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
	// double usage() const { return 1.0 * counter / n; }
	int hfn() const { return hf_num; }

private:
	mutable long long _cnt;
	int n, hf_num;
	int *pool;
	int counter;
	hash_func* hf;
};