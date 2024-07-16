#pragma once
#include <cstring>
#include <climits>
#include <algorithm>
#include "../common/hash.hpp"

#define INF 1000000000
// #define DEBUG_LESS_POOL

#define INDEX_LENGTH 6
#define INDEX_MAX 64
typedef uint8_t HLLT;

class hll_sketch{
public:
	hll_sketch(int memory, int hf_num, int shrink_flag = 0, int _pool_size = INDEX_MAX) 
		: total_memory(memory), pool_size(_pool_size){

		hash_func *hf = new hash_func[2];
		poolid_hf = hf[0];
		hll_hf = hf[1];
		
		pool_number = memory / (pool_size * sizeof(HLLT));
		if (shrink_flag) pool_number = pool_number / 64 * 64;
		#ifdef DEBUG_LESS_POOL
		pool_number = 1;
		#endif
		all_pool_array = new HLLT[pool_number * pool_size];
		memset(all_pool_array, 0, sizeof(HLLT) * pool_number * pool_size);
		all_pool = new HLLT*[pool_number];
		for (int i = 0; i < pool_number; i++){
			all_pool[i] = all_pool_array + i * pool_size;
		}

		for (int i = 0; i < 36; i++){
			lowbit_table[(1ll << i) % 37] = (HLLT)(i + 1);
		}
	}

	~hll_sketch(){
		delete[] all_pool;
		delete[] all_pool_array;
		// delete[] &(poolid_hf);
	}
	
	int total_memory;
	HLLT *all_pool_array;
	HLLT **all_pool;
	int pool_number, pool_size;
	hash_func poolid_hf, hll_hf;
	HLLT lowbit_table[37];
	mutable long long _cnt;

	void add(elem_t e, int offset = 0, int delta = 1) {
		// offset no use, all element in same sketch share same offset
		unsigned int pool_id = poolid_hf(e) % pool_number;
		unsigned int hllnumber = hll_hf(e);
		unsigned int index = hllnumber & (INDEX_MAX - 1);
		hllnumber = hllnumber >> INDEX_LENGTH;
		HLLT lowbit_where = lowbit_table[(hllnumber & -hllnumber) % 37];

		all_pool[pool_id][index] = max(lowbit_where, all_pool[pool_id][index]);
	}

	int query_pool(int pool_id){
		double constant, sum = 0;
		int m = INDEX_MAX;
		int zero_number = 0, result = 0;

		switch (INDEX_LENGTH) {
		case 4:
			constant = 0.673 * m * m;
		case 5:
			constant = 0.697 * m * m;
		case 6:
			constant = 0.709 * m * m;
		default:
			constant = (0.7213 / (1 + 1.079 / m)) * m * m;
		}

		for (int i = 0; i < INDEX_MAX; i++){
			if (all_pool[pool_id][i] == 0){
				zero_number++;
			}
			sum += 1.0 / (1ll << all_pool[pool_id][i]);
		}

		result = int(constant / sum);
		if (result < (5 / 2) * m)
    		result = (double)m * log((double)m / (double)zero_number);
		
		return result;
	}

	int query() {
		_cnt += hfn();
		int result = 0;
		for (int i = 0; i < pool_number; i++)
			result += query_pool(i);
		return result;
	}

	void shrink() {
		#ifdef DEBUG_LESS_POOL
		return;
		#endif
		total_memory /= 2;
		assert(pool_number % 2 == 0);
		pool_number /= 2;
		HLLT *new_all_pool_array = new HLLT[pool_number * pool_size];
		memset(new_all_pool_array, 0, sizeof(HLLT) * pool_number * pool_size);
		HLLT **new_all_pool = new HLLT*[pool_number];
		for (int i = 0; i < pool_number; i++){
			new_all_pool[i] = new_all_pool_array + i * pool_size;
		}

		for (int i = 0; i < pool_number; i++){
			for (int j = 0; j < INDEX_MAX; j++){
				new_all_pool[i][j] = max(all_pool[i][j], all_pool[i + pool_number][j]);
			}
		}
		delete[] all_pool_array;
		all_pool_array = new_all_pool_array;
		delete[] all_pool;
		all_pool = new_all_pool;
	}

	void clear() {
		poolid_hf.reset();
		hll_hf.reset(); // Use new hash functions
		memset(all_pool_array, 0, sizeof(HLLT) * pool_number * pool_size);
	}

	int size() const { return -1; }
	int memory() const { return total_memory; }

	long long qcnt() const { return _cnt; }
	// double usage() const { return 1.0 * counter / n; }
	int hfn() const { return (MEMORY_ACCESS_ALL) ? 1 : pool_number * pool_size; }
};