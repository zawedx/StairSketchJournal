#ifndef _LIGHT_PART_H_
#define _LIGHT_PART_H_

#include "param.h"
#include "../common/common.hpp"

// template<int init_mem_in_bytes>
class LightPart
{
	int counter_num;
	// static constexpr int counter_num = init_mem_in_bytes;
	BOBHash32 *bobhash = NULL;

public:
	uint8_t *counters;
	// uint8_t counters[counter_num];
	int mice_dist[256];

	LightPart(int _init_mem_in_bytes, int shrink_flag = 0): counter_num(_init_mem_in_bytes)
	{
		if (shrink_flag) counter_num = counter_num / 64 * 64;
		counters = new uint8_t[counter_num];
		clear();
		std::random_device rd;
       	bobhash = new BOBHash32(rd() % MAX_PRIME32);
	}
	~LightPart()
	{
		// printf("~LightPart()\n");
		delete[] counters;
		delete bobhash;
	}

	void clear()
	{
		memset(counters, 0, counter_num);
		memset(mice_dist, 0, sizeof(int) * 256);
	}


/* insertion */
	void insert(int wid, uint8_t *key, int f = 1)
	{
		char *conc_result = concatenate(wid, *((elem_t*)key));
		uint32_t hash_val = (uint32_t)bobhash->run(
			(const char*)conc_result,
			sizeof(int) + KEY_LENGTH
		);
		delete[] conc_result;
		uint32_t pos = hash_val % (uint32_t)counter_num;

		int old_val = (int)counters[pos];
        int new_val = (int)counters[pos] + f;

        new_val = new_val < 255 ? new_val : 255;
        counters[pos] = (uint8_t)new_val;

        mice_dist[old_val]--;
        mice_dist[new_val]++;
	}

	void swap_insert(int wid, uint8_t *key, int f)
	{
		char *conc_result = concatenate(wid, *((elem_t*)key));
		uint32_t hash_val = (uint32_t)bobhash->run(
			(const char*)conc_result,
			sizeof(int) + KEY_LENGTH
		);
		delete[] conc_result;
		uint32_t pos = hash_val % (uint32_t)counter_num;

        f = f < 255 ? f : 255;
        if (counters[pos] < f) 
        {
            int old_val = (int)counters[pos];
            counters[pos] = (uint8_t)f;
            int new_val = (int)counters[pos];

			mice_dist[old_val]--;
			mice_dist[new_val]++;
		}
	}


/* query */
    int query(int wid, elem_t e){
        uint8_t key[KEY_LENGTH];
        memcpy(key, &e, sizeof(elem_t));
        return query(wid, key);
    }

	int query(int wid, uint8_t *key) 
	{
		char *conc_result = concatenate(wid, *((elem_t*)key));
		uint32_t hash_val = (uint32_t)bobhash->run(
			(const char*)conc_result,
			sizeof(int) + KEY_LENGTH
		);
		delete[] conc_result;
        uint32_t pos = hash_val % (uint32_t)counter_num;

        return (int)counters[pos];
    }

// todo shrink max or plus? 
	void shrink() {
		assert(counter_num % 2 == 0);
		counter_num /= 2;
		uint8_t *new_pool = new uint8_t[counter_num];
		for (int i = 0; i < counter_num; ++i)
			new_pool[i] = max(counters[i], counters[i + counter_num]);
		delete[] counters;
		counters = new_pool;
	}


/* compress */
    void compress(int ratio, uint8_t *dst) 
    {
		int width = get_compress_width(ratio);

		for (int i = 0; i < width && i < counter_num; ++i) 
		{
			uint8_t max_val = 0;
			for (int j = i; j < counter_num; j += width)
                	max_val = counters[j] > max_val ? counters[j] : max_val;
			dst[i] = max_val;
        }
    }

	int query_compressed_part(uint8_t *key, uint8_t *compress_part, int compress_counter_num) 
	{
        uint32_t hash_val = (uint32_t)bobhash->run((const char *)key, KEY_LENGTH);
        uint32_t pos = (hash_val % (uint32_t)counter_num) % compress_counter_num;

        return (int)compress_part[pos];
    }


/* other measurement task */
    int get_compress_width(int ratio) { return (counter_num / ratio); }
    int get_compress_memory(int ratio) {	return (uint32_t)(counter_num / ratio); }
    int get_memory_usage() { return counter_num; }

   	// int get_cardinality() 
   	// {
	// 	int mice_card = 0;
    //     for (int i = 1; i < 256; i++)
	// 		mice_card += mice_dist[i];

	// 	double rate = (counter_num - mice_card) / (double)counter_num;
	// 	return counter_num * log(1 / rate);
    // }

    // void get_entropy(int &tot, double &entr)
    // {
    //     for (int i = 1; i < 256; i++) 
    //     {
    //         tot += mice_dist[i] * i;
	// 		entr += mice_dist[i] * i * log2(i);
	// 	}
    // }
};





#endif
