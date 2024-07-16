#pragma once
#include "../common/common.hpp"
#include "../common/hash.hpp"
#include "../common/utils.hpp"
#include <iostream>
#include "ElasticSketch.h"

class ada_elastic : public framework {
	ElasticSketch *sketch;
public:	
	/** 
	 * Create an Ada Elastic Sketch, use 
	 * @param memory Memory limit(Bytes) for Ada ElasticSketch
	 * @param hf_num Number of hash functions
	 */
	ada_elastic(int memory, int hf_num)
	:total_memory(memory){
		sketch = new ElasticSketch(memory);
	}

	~ada_elastic() override {
		delete sketch;
	}
	
	mutable long long _cnt;
    int bucket_number, bucket_length, item_num;
	int total_memory;

	hash_func_with_w* hf;

	/** 
	 * Add new elements to Ada ElasticSketch
	 * @param wid ID of window to add new elements
	 * @param e The new element
	 * @param delta The number of copies (of e) to add
	 */
	void add(int wid, elem_t e, int delta = 1) override {
		sketch->insert(wid, e, delta * wid);
	}

	pair<elem_t, int>** query_topk(pair<elem_t, int>** &result, int wid, int k = TOP_K) const override {
		pair<elem_t, int>** array_head = new pair<elem_t, int>*[2];
		result = array_head;
		sketch->query_topk(result[0], wid, k);
		for (int i = 0; i < k; i++)
			result[0][i].second /= wid;
		++result;
		return array_head;
	}

	pair<elem_t, int>** pretend_query_topk(pair<elem_t, int>** &result, int wid, int k = TOP_K) const {
		sketch->pretend_query_topk(*result, k);
		return nullptr;
	}

	pair<elem_t, int>** query_multiple_windows_topk(pair<elem_t, int>** &result, int wid_start, int wid_end, int k = TOP_K) const override {
		for (int i = wid_start; i <= wid_end; ++i) pretend_query_topk(result, i);
		return nullptr; 
	}

	bool add_delta_implemented() const override { return false; }

	int size() const { return -1; }
	int memory() const override { return -1; }

	long long qcnt() const override { return sketch->qcnt(); }
	string name() const override { return "AdaElastic"; }

private:
};
