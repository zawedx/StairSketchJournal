#pragma once
#include "common.hpp"

class framework {
public:

	/*
		Absolute Memory Config Format
			(int memory, int ds_num, int hf_num, int refresh_freq)
	*/
	framework(){}
	virtual ~framework(){}

	virtual int query(int wid, elem_t e) const { return 0; }

	virtual pair<elem_t, int>** query_topk(pair<elem_t, int>** &result, int wid, int k = 1000) const { return nullptr; }
	
	virtual int query_multiple_windows(int wid_start, int wid_end, elem_t e) const { return 0; }

	virtual pair<elem_t, int>** query_multiple_windows_topk(pair<elem_t, int>** &result, int wid_start, int wid_end, int k = 1000) const { return 0; }
	
	virtual void add(int wid, elem_t e, int delta = 1) {}

	virtual int memory() const { return 0; }

	virtual long long qcnt() const { return 0; }

	virtual bool add_delta_implemented() const { return false; }

	// query cardinal
	virtual int query(int wid) const { return 0; }

	virtual string name() const { return "framework"; }

private:
};