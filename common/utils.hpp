#pragma once

#include <cstdio>
#include <cstring>
#include <cassert>
#include <string>
#include <algorithm>
#include "common.hpp"
#include "hash.hpp"

class bit_set {
public:
	bit_set(int b) : bit_num(b), mem((b + 63) / 64) {
		bs = new u64[mem];
		memset(bs, 0, sizeof(u64) * mem);
	}
	~bit_set() { delete[] bs; }

	// Read index-th bit
	bool operator[](int i) const {
	#ifdef DEBUG
		assert(0 <= i && i < bit_num);
		assert((i >> 6) < mem);
	#endif
		return (bs[i >> 6] >> (i & 63) & 1);
	}

	// Set index-th bit to 1
	void set(int i) {
	#ifdef DEBUG
		assert(0 <= i && i < bit_num);
		assert((i >> 6) < bit_num);
	#endif
		bs[i >> 6] |= (1ull << (i & 63));
	}

	// Set all bits to 0
	void clear() { memset(bs, 0, sizeof(u64) * mem); }

	// Return the number of bits in bit_set
	int length() const { return bit_num; }

	int count() const {
		int cnt = 0;
		for (int i = 0; i < mem; ++i)
			cnt += __builtin_popcountll(bs[i]);
		return cnt;
	}

	int memory() const { return mem * 8; }

private:
	const int bit_num;
	const int mem;
	u64 *bs;
};

class HashMap {
public:
	static const int MOD = 10000009;
	elem_desc *hmap[MOD];
	unsigned char len[MOD];

	bool add(int i, elem_t e) {
		
		int idx = hf(e) % MOD;
		//fprintf(stderr, "!! %d\n");
		for (int k = 0; k < len[idx]; ++k)
			if (hmap[idx][k].e == e) {
				hmap[idx][k].cnt[i]++;
				return false;
			}
		len[idx]++;
		elem_desc *arr = new elem_desc[len[idx]];
		int* cnt = new int[cfg.win_num + 1]; 
		for (int k = 0; k <= cfg.win_num; ++k) cnt[k] = 0;
		cnt[i]++;
		arr[0] = elem_desc(e, cnt);
		for (int k = 1; k < len[idx]; ++k)
			arr[k] = hmap[idx][k - 1];
		if (len[idx] != 1) delete[] hmap[idx];
		hmap[idx] = arr;
		return true;
	}

	int all_elements(elem_desc* &elems) {
		int size = 0;
		for (int i = 0; i < MOD; ++i)
			size += len[i];

		elems = new elem_desc[size];
		int cur = 0;
		for (int i = 0; i < MOD; ++i)
			for (int k = 0; k < len[i]; ++k) {
				elems[cur] = hmap[i][k];
				elems[cur].cnt[0] = 0;
				for (int j = 1; j <= cfg.win_num; ++j)
					elems[cur].cnt[j] += elems[cur].cnt[j - 1];
				cur++;
			}
		return size;
	}
	
private:
	hash_func hf;
};