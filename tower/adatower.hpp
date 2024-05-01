#pragma once
#include "../common/common.hpp"
#include "../common/hash.hpp"
#include "../common/utils.hpp"
#include <iostream>
#include "tower.h"
#include "params.h"

class ada_tower : public framework {
public:	
	/** 
	 * Create an Ada tower Sketch, use 
	 * @param memory Memory limit(Bytes) for Ada towerS
	 * @param hf_num Number of hash functions
	 */
	ada_tower(int memory, int hf_num) {
		n = (memory >> 2) / d;
		sketch = new TowerSketch(memory, hf_num);
	}

	~ada_tower() override {
		delete sketch;
	}

	/** 
	 * Add new elements to Ada towerS
	 * @param wid ID of window to add new elements
	 * @param e The new element
	 * @param delta The number of copies (of e) to add
	 */
	void add(int wid, elem_t e, int delta = 1) override {
		char *new_key = concatenate(wid, e);
		uint16_t key_len = sizeof(int) + sizeof(elem_t);
		sketch->insert(new_key, key_len, delta * wid);
		delete[] new_key;
	}

	int query(int wid, elem_t e) const override {
		char *new_key = concatenate(wid, e);
		uint16_t key_len = sizeof(int) + sizeof(elem_t);
		int res = sketch->query(new_key, key_len);
		delete[] new_key;
		return res / wid;
	}

	int query_multiple_windows(int l, int r, elem_t e) const override {
		int sum = 0;
		for (int i = l; i <= r; ++i) sum += query(i, e);
		return sum;
	}

	bool add_delta_implemented() const override { return false; }

	int size() const { return n; }
	int memory() const override { return n * 4 * d; }

	long long qcnt() const override { return _cnt; }

private:
	mutable long long _cnt;
	int n, hf_num;
	hash_func* hf;
	TowerSketch *sketch;
};
