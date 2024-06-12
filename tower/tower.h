#ifndef _TOWER_H
#define _TOWER_H

#include <random>
#include <cstring>
#include <algorithm>

#include "params.h"
#include "murmur3.h"
#include "../common/common.hpp"

class TowerSketch
{
public:
	mutable long long _cnt;
    uint32_t w[d];
    uint32_t *A[d];
    uint32_t hashseed[d];
    int idx[d];

    TowerSketch() {}
    // TowerSketch(uint32_t w_d) { init(w_d); }
	TowerSketch(int memory, int hf_num, int shrink_flag = 0) {
        if (shrink_flag) init(((memory >> 2) / d) / 64 * 64);
        else init((memory >> 2) / d);
	}
    virtual ~TowerSketch() {
        for (int i = 0; i < d; ++i)
            delete[] A[i];
    }

    void init(uint32_t w_d)
    {
        std::random_device rd;
        for (int i = 0; i < d; ++i)
        {
            w[i] = w_d << 5 - i - 1;
            A[i] = new uint32_t[w_d];
            memset(A[i], 0, w_d * sizeof(uint32_t));
            hashseed[i] = rd() % MAX_PRIME32;
        }
    }

    void clear()
    {
        for (int i = 0; i < d; ++i)
            memset(A[i], 0, w[d-1] * sizeof(uint32_t));
    }

	void insert(int wid, elem_t e, int delta = 1) {
		char *new_key = concatenate(wid, e);
		uint16_t key_len = sizeof(int) + sizeof(elem_t);
		insert(new_key, key_len, delta);
		delete[] new_key;
	}

    virtual uint32_t insert(const char *key, uint16_t key_len, uint32_t delta = 1)
    {
        for (int i = 0; i < d; ++i)
            idx[i] = MurmurHash3_x86_32(key, key_len, hashseed[i]) % w[i];

        uint32_t ret = UINT32_MAX;
        for (int i = 0; i < d; ++i)
        {
            uint32_t &a = A[i][idx[i] >> cpw[i]];
            uint32_t shift = (idx[i] & lo[i]) << cs[i];
            uint32_t val = (a >> shift) & mask[i];
            a += (val < mask[i]) ? (std::min(delta, mask[i] - val) << shift) : 0;
            ret = (val < mask[i] && val < ret) ? val : ret;
        }
        return ret + delta;
    }

	uint32_t query(int wid, elem_t e) {
		char *new_key = concatenate(wid, e);
		uint16_t key_len = sizeof(int) + sizeof(elem_t);
		uint32_t result = query(new_key, key_len);
		delete[] new_key;
        return result;
	}

    uint32_t query(const char *key, uint16_t key_len)
    {
        _cnt += d;
        uint32_t ret = UINT32_MAX;
        for (int i = 0; i < d; ++i)
        {
            uint32_t idx = MurmurHash3_x86_32(key, key_len, hashseed[i]) % w[i];
            uint32_t a = A[i][idx >> cpw[i]];
            uint32_t shift = (idx & lo[i]) << cs[i];
            uint32_t val = (a >> shift) & mask[i];
            ret = (val < mask[i] && val < ret) ? val : ret;
        }
        return ret;
    }

	void shrink() {
		assert(w[d-1] % 2 == 0);
    
        for (int i = 0; i < d; ++i) w[i] /= 2;
        for (int i = 0; i < d; ++i){
            for (int j = 0; j < w[i]; j++){
                uint32_t &a1 = A[i][j >> cpw[i]];
                uint32_t &a2 = A[i][(j + w[i]) >> cpw[i]];
                uint32_t shift = (j & lo[i]) << cs[i];
                uint32_t val1 = (a1 >> shift) & mask[i];
                uint32_t val2 = (a2 >> shift) & mask[i];
                if (val1 < val2) a1 ^= (val1 ^ val2) << shift;
            }
        }
		for (int i = 0; i < d; ++i){
            uint32_t *new_A = new uint32_t[w[d-1]];
            memcpy(new_A, A[i], w[d-1] * sizeof(uint32_t));
            delete[] A[i];
            A[i] = new_A;
        }
	}

	long long qcnt() const { return _cnt; }
	long long hfn() const { return d; }
};

class TowerSketchCU : public TowerSketch
{
public:
    TowerSketchCU() {}
    TowerSketchCU(uint32_t w_d) { init(w_d); }
    ~TowerSketchCU() {}

    uint32_t insert(const char *key, uint16_t key_len)
    {
        uint32_t min_val = UINT32_MAX;
        for (int i = 0; i < d; ++i)
            idx[i] = MurmurHash3_x86_32(key, key_len, hashseed[i]) % w[i];
        for (int i = 0; i < d; ++i)
        {
            uint32_t a = A[i][idx[i] >> cpw[i]];
            uint32_t shift = (idx[i] & lo[i]) << cs[i];
            uint32_t val = (a >> shift) & mask[i];
            min_val = (val < mask[i] && val < min_val) ? val : min_val;
        }
        for (int i = 0; i < d; ++i)
        {
            uint32_t &a = A[i][idx[i] >> cpw[i]];
            uint32_t shift = (idx[i] & lo[i]) << cs[i];
            uint32_t val = (a >> shift) & mask[i];
            a += (val < mask[i] && val == min_val) ? (1 << shift) : 0;
        }
        return min_val + 1;
    }
};

class TowerSketchHalfCU : public TowerSketch
{
public:
    TowerSketchHalfCU() {}
    TowerSketchHalfCU(uint32_t w_d) { init(w_d); }
    ~TowerSketchHalfCU() {}

    uint32_t insert(const char *key, uint16_t key_len)
    {
        uint32_t min_val = UINT32_MAX;
        for (int i = 0; i < d; ++i)
            idx[i] = MurmurHash3_x86_32(key, key_len, hashseed[i]) % w[i];
        for (int i = 0; i < d; ++i)
        {
            uint32_t &a = A[i][idx[i] >> cpw[i]];
            uint32_t shift = (idx[i] & lo[i]) << cs[i];
            uint32_t val = (a >> shift) & mask[i];
            if (val < mask[i] && val <= min_val)
            {
                a += 1 << shift;
                min_val = val;
            }
        }
        return min_val + 1;
    }
};

#endif