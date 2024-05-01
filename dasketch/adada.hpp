#pragma once
#include "../common/common.hpp"
#include "../common/hash.hpp"
#include "../common/utils.hpp"
#include <iostream>
#include "csm_sketch.hpp"

class ada_da : public framework {
public:	
	/** 
	 * Create an Ada DA Sketch, use 
	 * @param memory Memory limit(Bytes) for Ada DAS
	 * @param hf_num Number of hash functions
	 */
	ada_da(int memory, int hf_num, double heavy_rate = 0.8, double alpha = 1, double cons = 1. / 1.08) 
		: total_memory(memory), heavy_rate(heavy_rate), alpha(alpha), cons(cons), hf(new hash_func_with_w[1]) {

		light_part = new csm_sketch(1.0 * memory * (1 - heavy_rate), hf_num);

		item_num = int(1.0 * memory * heavy_rate / sizeof(Item));
		bucket_length = 8;
		bucket_number = (item_num - 1) / bucket_length + 1;
		item_num = bucket_length * bucket_number;
		item_arr = new Item[item_num];
		item = new Item*[bucket_number];
		for (int i=0; i<bucket_number; i++)
			item[i] = item_arr + i * bucket_length;
		memset(item_arr, 0, sizeof(Item) * item_num);
		
	}

	~ada_da() override {
		delete light_part;
		delete[] item_arr;
		delete[] item;
	}
	
	double heavy_rate, cons, alpha;
    int bucket_number, bucket_length, item_num;
	int total_memory;

	class Item{
	public:
		ItemInfo id;
		int cnt;
		int underest;
		int store_unbiased;
		int store_overest;
	};
	Item **item, *item_arr;

	csm_sketch* light_part;
	hash_func_with_w* hf;

	// bool is_topk[MAX_bucketNum][64]; // data structures convenient for query
	class Query_Item: public Item{
	public:
		int buc_id;
		int pos;

		bool operator < (const Query_Item& other) const{
			return cnt > other.cnt;
		}
	};

	/** 
	 * Add new elements to Ada DAS
	 * @param wid ID of window to add new elements
	 * @param e The new element
	 * @param delta The number of copies (of e) to add
	 */
	void add(int wid, elem_t e, int delta = 1) override {
		ItemInfo e_with_w(e, wid);
		int offset = wid;
		unsigned int h_id = (hf[0](e_with_w)) % bucket_number;

		int i, j;
		int min_cnt = INF, arg_min = -1;

		for (j = 0; j < bucket_length; j++){
			if ((item[h_id][j].id == e_with_w) && (item[h_id][j].underest >= 1)){
				item[h_id][j].cnt += delta;
				item[h_id][j].underest += delta;
				return;
			}
		}

		for (j = 0; j < bucket_length; j++){
			if (min_cnt > item[h_id][j].cnt){
				min_cnt = item[h_id][j].cnt;
				arg_min = j;
			}
		}


		for (j = 0; j < bucket_length; j++){
			if (item[h_id][j].cnt == 0){
				item[h_id][j].id = e_with_w;
				item[h_id][j].cnt = delta;
				item[h_id][j].underest = delta;
				item[h_id][j].store_unbiased = light_part->query(e, offset, 1) / wid;
				item[h_id][j].store_overest = light_part->query(e, offset, 0) / wid;
				return;
			}
		}

		if(rand() % 100000 < alpha * 100000 *  1. / (double)(item[h_id][arg_min].cnt + 1)){
			light_part->add(
				item[h_id][arg_min].id.id, 
				item[h_id][arg_min].id.w, 
				item[h_id][arg_min].underest * item[h_id][arg_min].id.w
			);
			item[h_id][arg_min].underest = delta;
			item[h_id][arg_min].store_unbiased = light_part->query(e, offset, 1) / wid;
			item[h_id][arg_min].store_overest = light_part->query(e, offset, 0) / wid;
			item[h_id][arg_min].id = e_with_w;
		}
		else{
			light_part->add(e, offset, delta * wid);
		}
	}

	static bool greater_pair(const pair<elem_t, int> &x, const pair<elem_t, int> &y){
		return x.second > y.second;
	}

	pair<elem_t, int>** query_topk(pair<elem_t, int>** &result, int wid, int k = 1000) const override {
		pair<elem_t, int>** array_head = new pair<elem_t, int>*[2];
		result = array_head;
		*result = new pair<elem_t, int>[k];
		vector<pair<elem_t, int> > all_possible_topk;
		all_possible_topk.clear();
		for (int i = 0; i < bucket_number; i++)
			for (int j = 0; j < bucket_length; j++){
				if (item[i][j].id.w == wid){
					all_possible_topk.push_back(make_pair(item[i][j].id.id, item[i][j].store_unbiased));
				}
			}
		sort(all_possible_topk.begin(), all_possible_topk.end(), greater_pair);
		for (int i = 0; i < k; i++)
			result[0][i] = all_possible_topk[i];
		++result;
		return array_head;
	}

	bool add_delta_implemented() const override { return false; }

	int size() const { return -1; }
	int memory() const override { return -1; }

	long long qcnt() const override { return -1; }

private:
};
