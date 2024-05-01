#pragma once
#include <cstring>
#include <climits>
#include <algorithm>
#include "../common/hash.hpp"
#include "csm_sketch.hpp"

#define INF 1000000000

class da_sketch{
public:
	da_sketch(int memory, int hf_num, int shrink_flag = 0, double heavy_rate = 0.8, double alpha = 1, double cons = 1. / 1.08) 
		: total_memory(memory), heavy_rate(heavy_rate), alpha(alpha), cons(cons), hf(new hash_func[1]) {

		light_part = new csm_sketch(1.0 * memory * (1 - heavy_rate), hf_num, shrink_flag);

		item_num = int(1.0 * memory * heavy_rate / sizeof(Item));
		bucket_length = 8;
		bucket_number = (item_num - 1) / bucket_length + 1;
		if (shrink_flag) bucket_number = bucket_number / 64 * 64;
		item_num = bucket_length * bucket_number;
		item_arr = new Item[item_num];
		item = new Item*[bucket_number];
		for (int i=0; i<bucket_number; i++)
			item[i] = item_arr + i * bucket_length;
		memset(item_arr, 0, sizeof(Item) * item_num);
		
	}
	
	double heavy_rate, cons, alpha;
    int bucket_number, bucket_length, item_num;
	int total_memory;

	class Item{
	public:
		elem_t id;
		int cnt;
		int underest;
		int store_unbiased;
		int store_overest;
	};
	Item **item, *item_arr;

	csm_sketch* light_part;
	hash_func* hf;

	// bool is_topk[MAX_bucketNum][64]; // data structures convenient for query
	class Query_Item: public Item{
	public:
		int buc_id;
		int pos;

		bool operator < (const Query_Item& other) const{
			return cnt > other.cnt;
		}
	};

	void add(elem_t e, int offset = 0, int delta = 1) {
		unsigned int h_id = (hf[0](e) + offset) % bucket_number;

		int i, j;
		int min_cnt = INF, arg_min = -1;

		for (j = 0; j < bucket_length; j++){
			if ((item[h_id][j].id == e) && (item[h_id][j].underest >= 1)){
				item[h_id][j].cnt++;
				item[h_id][j].underest++;
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
				item[h_id][j].id = e;
				item[h_id][j].cnt = 1;
				item[h_id][j].underest = 1;
				item[h_id][j].store_unbiased = light_part->query(e, offset, 1);
				item[h_id][j].store_overest = light_part->query(e, offset, 0);
				return;
			}
		}

		if(rand() % 100000 < alpha * 100000 *  1. / (double)(item[h_id][arg_min].cnt + 1)){
			light_part->add(item[h_id][arg_min].id, offset, item[h_id][arg_min].underest);
			item[h_id][arg_min].underest = 1;
			item[h_id][arg_min].store_unbiased = light_part->query(e, offset, 1);
			item[h_id][arg_min].store_overest = light_part->query(e, offset, 0);
			item[h_id][arg_min].id = e;
		}
		else{
			light_part->add(e, offset, delta);
		}
	}

	int query(elem_t e, int offset = 0) const {
		// _cnt += hf_num;
		// int res = INT_MAX;
		// for (int i = 0; i < hf_num; ++i)
		// 	res = std::min(res, pool[(hf[i](e) + offset) % n]);
		// return res;
		return -1;
	}

	void query_topk(pair<elem_t, int> *result, int k = 1000) const {
		Query_Item* query_item = new Query_Item[bucket_number * bucket_length];

		for (int i = 0; i < bucket_number; i++){
			for (int j = 0; j < bucket_length; j++){
				query_item[i * bucket_length + j].buc_id = i;
				query_item[i * bucket_length + j].pos = j;
				query_item[i * bucket_length + j].cnt = item[i][j].cnt;
			}
		}

		sort(query_item, query_item + item_num);
		
		for (int i = 0; i < min(k, item_num); i++){
			int tmp;
			Item &it = item[query_item[i].buc_id][query_item[i].pos];
			result[i] = make_pair(it.id, it.underest + it.store_unbiased);
		}
		for (int i = item_num; i < k; i++){
			result[i] = make_pair(elem_t(rand()), -1);
		}
		
		delete(query_item);
	}

	void shrink() {
		assert(bucket_number % 2 == 0);
		total_memory /= 2;
		item_num /= 2;
		bucket_length = 8;
		bucket_number /= 2;
		item_num = bucket_length * bucket_number;
		Item *new_item_arr = new Item[item_num];
		Item **new_item = new Item*[bucket_number];
		for (int i=0; i<bucket_number; i++)
			new_item[i] = new_item_arr + i * bucket_length;
		for (int i=0; i<bucket_number; i++)
			for (int j=0; j<bucket_length; j++){
				// compare item[i][j] item[i+bucket_number][j]
				Item *it1 = &item[i][j];
				Item *it2 = &item[i + bucket_number][j];
				if (it1->underest < it2->underest) swap(it1, it2);
				new_item[i][j] = *it1;
				light_part->add(it2->id, 0, it2->underest);
			}
		light_part->shrink();
	}

	void clear() {
		hf[0].reset(); // Use new hash functions
		memset(item_arr, 0, sizeof(Item) * item_num);
	}

	int size() const { return total_memory / 4; }
	int memory() const { return total_memory; }

	long long qcnt() const { return 100; } //_cnt; } // to be fixed
	// double usage() const { return 1.0 * counter / n; }
	int hfn() const { return light_part->hfn(); }
};