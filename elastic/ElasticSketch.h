#ifndef _ELASTIC_SKETCH_H_
#define _ELASTIC_SKETCH_H_

#include "HeavyPart-noSIMD.h"
#include "LightPart.h"
#include "../common/common.hpp"


// template<int bucket_num, int tot_memory_in_bytes>
class ElasticSketch
{
    int bucket_num, tot_memory_in_bytes;
    int heavy_mem, light_mem;
	mutable long long _cnt;

    HeavyPart *heavy_part;
    LightPart *light_part;
    // HeavyPart<bucket_num> heavy_part;
    // LightPart<light_mem> light_part;

public:
    ElasticSketch(int _tot_memory_in_bytes, int shrink_flag = 0)
    :tot_memory_in_bytes(_tot_memory_in_bytes){
        double heavy_ratio = 0.8;
        heavy_mem = tot_memory_in_bytes * heavy_ratio;
        light_mem = tot_memory_in_bytes - heavy_mem;
        bucket_num = heavy_mem / sizeof(Bucket);
        // to shrink
        if (shrink_flag) bucket_num = bucket_num / 64 * 64;
        heavy_mem = bucket_num * sizeof(Bucket);
        light_mem = tot_memory_in_bytes - heavy_mem;
        heavy_part = new HeavyPart(bucket_num);
        light_part = new LightPart(light_mem, shrink_flag);
    }
    // ElasticSketch(int _bucket_num, int _tot_memory_in_bytes)
    //     : bucket_num(_bucket_num), tot_memory_in_bytes(_tot_memory_in_bytes){
    //     heavy_mem = bucket_num * sizeof(Bucket);
    //     light_mem = tot_memory_in_bytes - heavy_mem;
    //     heavy_part = new HeavyPart(bucket_num);
    //     light_part = new LightPart(light_mem);
    // }
    ~ElasticSketch(){
		// printf("~ElasticSketch()\n");
        delete heavy_part;
        delete light_part;
    }
    void clear()
    {
        heavy_part->clear();
        light_part->clear();
    }

    void insert(int wid, elem_t e, int f = 1){
        uint8_t key[KEY_LENGTH];
        memcpy(key, &e, sizeof(elem_t));
        insert(wid, key, f);
    }

    void insert(int wid, uint8_t *key, int f = 1)
    {
        uint8_t swap_key[KEY_LENGTH];
        uint32_t swap_val = 0;
        int swap_wid = 0;
        int result = heavy_part->insert(wid, key, swap_key, swap_val, swap_wid, f);

        switch(result)
        {
            case 0: return;
            case 1:{
                if(HIGHEST_BIT_IS_1(swap_val))
                    light_part->insert(swap_wid, swap_key, GetCounterVal(swap_val));
                else
                    light_part->swap_insert(swap_wid, swap_key, swap_val);
                return;
            }
            case 2: light_part->insert(wid, key, f);  return;
            default:
                printf("error return value !\n");
                exit(1);
        }
    }

    void prepare_topk(vector<pair<elem_t, uint32_t> > &all_possible_topk, int wid){
        all_possible_topk.clear();
        heavy_part->prepare_topk(all_possible_topk, wid);
        for (int i = 0; i < all_possible_topk.size(); i++){
            // if(HIGHEST_BIT_IS_1(all_possible_topk[i].second))
            uint32_t heavy_result = all_possible_topk[i].second;
            if(heavy_result == 0 || HIGHEST_BIT_IS_1(heavy_result))
            {
                int light_result = light_part->query(wid, all_possible_topk[i].first);
                all_possible_topk[i].second = GetCounterVal(heavy_result) + (uint32_t)light_result;
            }
        }
    }

    void query_topk(pair<elem_t, int>* &result, int wid, int k = 1000){
        _cnt += hfn();
        vector<pair<elem_t, uint32_t> > all_possible_topk;
        prepare_topk(all_possible_topk, wid);
        sort(all_possible_topk.begin(), all_possible_topk.end(), sortBySecondDesc);
        result = new pair<elem_t, int>[k];
        for (int i = 0; i < k; i++)
            if (i >= all_possible_topk.size())
                result[i] = make_pair((elem_t)0, (int)-1);
            else 
                result[i] = make_pair(
                    all_possible_topk[i].first, 
                    (int)all_possible_topk[i].second
                );
    }

    // void quick_insert(uint8_t *key, int f = 1)
    // {
    //     heavy_part->quick_insert(key, f);
    // }

    int query(int wid, elem_t e){
        uint8_t key[KEY_LENGTH];
        memcpy(key, &e, sizeof(elem_t));
        return query(wid, key);
    }

    int query(int wid, uint8_t *key)
    {
        uint32_t heavy_result = heavy_part->query(wid, key);
        if(heavy_result == 0 || HIGHEST_BIT_IS_1(heavy_result))
        {
            int light_result = light_part->query(wid, key);
            return (int)GetCounterVal(heavy_result) + light_result;
        }
        return heavy_result;
    }
	
	void pretend_query_topk(pair<elem_t, int> *result, int k = 1000) const {
		_cnt += hfn();
	}


	void shrink() {
        vector<Bucket> shrink_insert;
        shrink_insert.clear();
        heavy_part->shrink(shrink_insert);
        for (int i = 0; i < shrink_insert.size(); i++)
            for (int j = 0; j < MAX_VALID_COUNTER; j++){
                insert(shrink_insert[i].wid[j], shrink_insert[i].key[j], GetCounterVal(shrink_insert[i].val[j]));
            }
        light_part->shrink();
	}

    // int query_compressed_part(uint8_t *key, uint8_t *compress_part, int compress_counter_num)
    // {
    //     uint32_t heavy_result = heavy_part->query(key);
    //     if(heavy_result == 0 || HIGHEST_BIT_IS_1(heavy_result))
    //     {
    //         int light_result = light_part->query_compressed_part(key, compress_part, compress_counter_num);
    //         return (int)GetCounterVal(heavy_result) + light_result;
    //     }
    //     return heavy_result;
    // }

    // void get_heavy_hitters(int threshold, vector<pair<string, int>> & results)
    // {
    //     for (int i = 0; i < bucket_num; ++i) 
    //         for (int j = 0; j < MAX_VALID_COUNTER; ++j) 
    //         {
    //             uint32_t key = heavy_part->buckets[i].key[j];
    //             int val = query((uint8_t *)&key);
    //             if (val >= threshold) {
    //                 results.push_back(make_pair(string((const char*)&key, 4), val));
    //             }
    //         }
    // }

/* interface */
    // int get_compress_width(int ratio) { return light_part->get_compress_width(ratio);}
    // void compress(int ratio, uint8_t *dst) {    light_part->compress(ratio, dst); }
    int get_bucket_num() { return heavy_part->get_bucket_num(); }
    int memory() { return tot_memory_in_bytes; }
    long long qcnt() const { return _cnt; }
	int hfn() const { return ((MEMORY_ACCESS_ALL) ? 1 : bucket_num) + 1/* light part */; }
    // double get_bandwidth(int compress_ratio) 
    // {
    //     int result = heavy_part->get_memory_usage();
    //     result += get_compress_width(compress_ratio) * sizeof(uint8_t);
    //     return result * 1.0 / 1024 / 1024;
    // }

    // int get_cardinality()
    // {
    //     int card = light_part->get_cardinality();
    //     for(int i = 0; i < bucket_num; ++i)
    //         for(int j = 0; j < MAX_VALID_COUNTER; ++j)
    //         {
    //             uint8_t key[KEY_LENGTH_4];
    //             *(uint32_t*)key = heavy_part->buckets[i].key[j];
    //             int val = heavy_part->buckets[i].val[j];
    //             int ex_val = light_part->query(key);

    //             if(HIGHEST_BIT_IS_1(val) && ex_val)
    //             {
    //                 val += ex_val;
    //                 card--;
    //             }
    //             if(GetCounterVal(val))
    //                 card++;
    //         }
    //         return card;
    // }

    // double get_entropy()
    // {
    //     int tot = 0;
    //     double entr = 0;

    //     light_part->get_entropy(tot, entr);

    //     for(int i = 0; i < bucket_num; ++i)
    //         for(int j = 0; j < MAX_VALID_COUNTER; ++j)
    //         {
    //             uint8_t key[KEY_LENGTH_4];
    //             *(uint32_t*)key = heavy_part->buckets[i].key[j];
    //             int val = heavy_part->buckets[i].val[j];

    //             int ex_val = light_part->query(key);

    //             if(HIGHEST_BIT_IS_1(val) && ex_val)
    //             {
    //                 val += ex_val;

    //                 tot -= ex_val;

    //                 entr -= ex_val * log2(ex_val);
    //             }
    //             val = GetCounterVal(val);
    //             if(val)
    //             {
    //                 tot += val;
    //                 entr += val * log2(val);
    //             }
    //         }
    //     return -entr / tot + log2(tot);
    // }

    // void get_distribution(vector<double> &dist)
    // {
    //     light_part->get_distribution(dist);

    //     for(int i = 0; i < bucket_num; ++i)
    //         for(int j = 0; j < MAX_VALID_COUNTER; ++j)
    //         {
    //             uint8_t key[KEY_LENGTH_4];
    //             *(uint32_t*)key = heavy_part->buckets[i].key[j];
    //             int val = heavy_part->buckets[i].val[j];

    //             int ex_val = light_part->query(key);

    //             if(HIGHEST_BIT_IS_1(val) && ex_val != 0)
    //             {
    //                 val += ex_val;
    //                 dist[ex_val]--;
    //             }
    //             val = GetCounterVal(val);
    //             if(val)
    //             {
    //                 if(val + 1 > dist.size())
    //                     dist.resize(val + 1);
    //                 dist[val]++;
    //             }
    //         }
    // }

    // void *operator new(size_t sz)
    // {
    //     constexpr uint32_t alignment = 64;
    //     size_t alloc_size = (2 * alignment + sz) / alignment * alignment;
    //     void *ptr = ::operator new(alloc_size);
    //     void *old_ptr = ptr;
    //     void *new_ptr = ((char*)std::align(alignment, sz, ptr, alloc_size) + alignment);
    //     ((void **)new_ptr)[-1] = old_ptr;

    //     return new_ptr;
    // }
    // void operator delete(void *p)
    // {
    //     ::operator delete(((void**)p)[-1]);
    // }
};



#endif
