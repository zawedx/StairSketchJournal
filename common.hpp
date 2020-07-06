#pragma once
#include <vector>
#include <unordered_set>
#include <unordered_map>

typedef unsigned u32;
typedef unsigned long long u64;

typedef unsigned long long elem_t;

struct interval {
	int l, r;
	interval() { l = r = 0; }
	interval(int l, int r) : l(l), r(r) {}
};

struct elem_desc {
	elem_t e;
	int *cnt;

	elem_desc() { e = 0; cnt = nullptr; }
	elem_desc(elem_t e, int *cnt) : e(e), cnt(cnt) {}
};

elem_desc *elems = nullptr;
int elem_cnt;

std::vector<elem_t> win_data[300];
std::unordered_map<elem_t, int> win_set[300];
std::unordered_set<elem_t> elem_set;