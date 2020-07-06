#pragma once
#include "common.hpp"
#include "utils.hpp"

class bloom_filter {
public:	
	bloom_filter(int mem, int hf_num) 
		: n((mem / 8) * 64), hf_num(hf_num), bs(new bit_set(n)), hf(new hash_func[hf_num]) {
		_cnt = 0;
	}

	void add(elem_t e, int offset = 0) {
		for (int i = 0; i < hf_num; ++i)
			bs->set((hf[i](e) + offset) % n);
	}

	bool query(elem_t e, int offset = 0) const {
		_cnt += hf_num;
		bool ret = true;
		for (int i = 0; i < hf_num; ++i)
			if (!(*bs)[(hf[i](e) + offset) % n])
				ret = false;
		return ret;
	}

	void shrink() {
		n /= 2;
	#ifdef DEBUG
		assert(n % 64 == 0);
	#endif
		bit_set *new_bs = new bit_set(n);
		for (int i = 0; i < n; ++i)
			if ((*bs)[i] || (*bs)[i + n])
				new_bs->set(i);
		delete bs;
		bs = new_bs;
	}

	void copy(bloom_filter *bf) {
		bs->clear();
		for (int i = 0; i < n; ++i)
			if ((*bf->bs)[i])
				bs->set(i);
		for (int i = 0; i < hf_num; ++i)
			hf[i] = bf->hf[i];
	}

	void merge(bloom_filter *bf) {
		#ifdef DEBUG
			assert(bf->n == n);
		#endif
		for (int i = 0; i < n; ++i)
			if ((*bf->bs)[i])
				bs->set(i);
	}

	void clear(bool reset_hf = true) {
		if (reset_hf) {
			for (int i = 0; i < hf_num; ++i)
				hf[i].reset();
		}
		bs->clear();
	}

	int memory() const { return bs->memory(); }
	long long qcnt() const { return _cnt; }	
	int hfn() const { return hf_num; }

private:
	mutable long long _cnt;
	int n, hf_num;
	bit_set* bs;
	hash_func* hf;
};