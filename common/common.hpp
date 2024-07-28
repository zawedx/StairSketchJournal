#pragma once
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <assert.h>
#include <algorithm>
#include <cstring>

using namespace std;

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

bool sortBySecondDesc(const std::pair<elem_t, int> &a, const std::pair<elem_t, int> &b) {
    return a.second > b.second;
}

template <typename T1, typename T2>
char* concatenate(const T1& val1, const T2& val2) {
    size_t size1 = sizeof(T1);
    size_t size2 = sizeof(T2);
    char* result = new char[size1 + size2];
    std::memcpy(result, &val1, size1);
    std::memcpy(result + size1, &val2, size2);
    return result;
}

template <typename T1>
char* convert(const T1& val1) {
    size_t size1 = sizeof(T1);
    char* result = new char[size1];
    std::memcpy(result, &val1, size1);
    return result;
}

#define MEMORY_ACCESS_ALL 0

void delete_topk_result(pair<elem_t, int>** answer_begin, pair<elem_t, int>** answer_end){
	for (pair<elem_t, int> **it = answer_begin; it != answer_end; it++)
		delete[] *it;
	delete[] answer_begin;
}

#define TOP_K 1000

#define DA_DEFAULT_MEMORY MB(10)
#define ELASTIC_DEFAULT_MEMORY MB(20)
#define TOWER_DEFAULT_MEMORY MB(1)
#define CMCU_DEFAULT_MEMORY MB(60)

#define MAX_WINNUMBER 256