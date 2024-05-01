#pragma once
#include "../common/common.hpp"
#include "../common/hash.hpp"
#include "../common/utils.hpp"
#include <iostream>

class ada_cm {
public:	
	/** 
	 * Create an Ada CM Sketch, use 
	 * @param memory Memory limit(Bytes) for Ada CMS
	 * @param hf_num Number of hash functions
	 */
	ada_cm(int memory, int hf_num) 
		: n(memory >> 2), hf_num(hf_num), hf(new hash_func[hf_num]) {
		pool = new int[n];
		memset(pool, 0, sizeof(int) * n);
	}

	~ada_cm() {
		delete[] pool;
		delete[] hf;
	}

	/** 
	 * Add new elements to Ada CMS
	 * @param wid ID of window to add new elements
	 * @param e The new element
	 * @param delta The number of copies (of e) to add
	 */
	void add(int wid, elem_t e, int delta = 1) {
		for (int i = 0; i < hf_num; ++i) 
			pool[(hf[i](e) + wid) % n] += wid * delta;
		++counter;
	}

	int query(int wid, elem_t e) const {
		_cnt += hf_num;
		int res = INT_MAX;
		for (int i = 0; i < hf_num; ++i)
			res = std::min(res, pool[(hf[i](e) + wid) % n]);
		return res / wid;
	}

	int query_multiple_windows(int l, int r, elem_t e) {
		int sum = 0;
		for (int i = l; i <= r; ++i) sum += query(i, e);
		return sum;
	}

	bool add_delta_implemented() const { return true; }

	int size() const { return n; }
	int memory() const { return n * 4; }

	long long qcnt() const { return _cnt; }

private:
	mutable long long _cnt;
	int n, hf_num;
	int *pool;
	int counter;
	hash_func* hf;
};
