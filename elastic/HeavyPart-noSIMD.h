#ifndef _HEAVYPART_H_
#define _HEAVYPART_H_

#include "param.h"
#include "../common/common.hpp"
#include "../common/hash.hpp"




// template<int bucket_num>
class HeavyPart
{
	int bucket_num;
public:
	Bucket *buckets;
	// alignas(64) Bucket buckets[bucket_num];
	hash_func* hf;

	HeavyPart(int _bucket_num): bucket_num(_bucket_num), hf(new hash_func[1])
	{
		buckets = new Bucket[bucket_num];
		clear();
	}
	~HeavyPart(){
		// printf("~HeavyPart()\n");
		delete[] buckets;
	}

	void clear()
	{
		hf[0].reset();
		memset(buckets, 0, sizeof(Bucket) * bucket_num);
	}

/* insertion */
	int insert(int wid, uint8_t *key, uint8_t *swap_key, uint32_t &swap_val, int &swap_wid, uint32_t f = 1)
	{
		elem_t fp;
		int pos = CalculateFP(wid, key, fp);

		/* find if there has matched bucket */
		int matched = -1, empty = -1, min_counter = 0;
		uint32_t min_counter_val = GetCounterVal(buckets[pos].val[0]);
		for(int i = 0; i < COUNTER_PER_BUCKET - 1; i++)
		{
			if(buckets[pos].key[i] == fp && buckets[pos].wid[i] == wid){
				matched = i;
				break;
			}
			if(buckets[pos].key[i] == 0 && empty == -1)
				empty = i;
			if(min_counter_val > GetCounterVal(buckets[pos].val[i])){
				min_counter = i;
				min_counter_val = GetCounterVal(buckets[pos].val[i]);
			}
		}

		/* if matched */
		if(matched != -1){
			buckets[pos].val[matched] += f;
			return 0;
		}

		/* if there has empty bucket */
		if(empty != -1){
			buckets[pos].key[empty] = fp;
			buckets[pos].val[empty] = f;
			buckets[pos].wid[empty] = wid;
			return 0;
		}

		/* update guard val and comparison */
		uint32_t guard_val = buckets[pos].val[MAX_VALID_COUNTER];
		// for time adaptive
		// guard_val = UPDATE_GUARD_VAL(guard_val);
		guard_val = guard_val + f;

		if(!JUDGE_IF_SWAP(GetCounterVal(min_counter_val), guard_val))
		{
			buckets[pos].val[MAX_VALID_COUNTER] = guard_val;
			return 2;
		}

	
		*((elem_t*)swap_key) = buckets[pos].key[min_counter];
		swap_val = buckets[pos].val[min_counter];
		swap_wid = buckets[pos].wid[min_counter];


		buckets[pos].val[MAX_VALID_COUNTER] = 0;


		buckets[pos].key[min_counter] = fp;
		buckets[pos].val[min_counter] = 0x80000000 | f;
		// buckets[pos].val[min_counter] = 0x80000001;
		// assert(f == 1);
		buckets[pos].wid[min_counter] = wid;

		return 1;
	}





/* query */
	uint32_t query(int wid, uint8_t *key)
	{
		elem_t fp;
		int pos = CalculateFP(wid, key, fp);

		for(int i = 0; i < MAX_VALID_COUNTER; ++i)
			// if(buckets[pos].key[i] == fp)
			// 	return buckets[pos].val[i];
			if (buckets[pos].key[i] == fp && buckets[pos].wid[i] == wid)
				return buckets[pos].val[i];
		return 0;
	}

	
    void prepare_topk(vector<pair<elem_t, uint32_t> > &all_possible_topk, int wid){
		for (int i = 0; i < bucket_num; i++)
			for (int j = 0; j < MAX_VALID_COUNTER; j++) if (buckets[i].wid[j] == wid){
				all_possible_topk.push_back(make_pair(buckets[i].key[j], buckets[i].val[j]));
			}
    }

	void shrink(vector<Bucket> &shrink_kick){
		assert(bucket_num % 2 == 0);
		bucket_num /= 2;
		for (int i = bucket_num; i < 2 * bucket_num; i++)
			shrink_kick.push_back(buckets[i]);
		// Bucket *new_buckets = new Bucket[bucket_num];
		// memcpy(new_buckets, buckets, sizeof(Bucket) * bucket_num);
		// delete[] buckets;
		// buckets = new_buckets;
	}


/* interface */
	int get_memory_usage()
	{
		return bucket_num * sizeof(Bucket);
	}
	int get_bucket_num()
	{
		return bucket_num;
	}

private:
	int CalculateFP(int wid, uint8_t *key, elem_t &fp)
	{
		fp = *((elem_t*)key);
		return (hf[0](fp) + wid) % bucket_num;
		// return (CalculateBucketPos(fp) % bucket_num + wid) % bucket_num;
	}
};

#endif












