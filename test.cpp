#include "impl_all.h"

template<class Sketch>
void build_sketch(Sketch *sketch) {
	if (sketch->add_delta_implemented()) {
		for (int i = 1; i <= cfg.win_num; ++i) 
			for (int k = 0; k < elem_cnt; ++k)
				if (elems[k].cnt[i] - elems[k].cnt[i-1] > 0)
					sketch->add(i, elems[k].e, elems[k].cnt[i] - elems[k].cnt[i-1]);
	} else {
		for (int i = 1; i <= cfg.win_num; ++i)
			for (elem_t e : win_data[i])
				sketch->add(i, e);
	}
}

vector<tuple<int, int, int, int> > stair_config(int memory, int k) {
	int sum = 0;
	for (int i = 0; i <= k; ++i) 
		sum += (1 << i) * (i == k ? 4 : 1);

	double unit = (double) memory / sum;
	vector<tuple<int, int, int, int> > ds_config = 
		{ make_tuple(unit, 1, 1, 1) };

	for (int i = 1; i <= k; ++i)
		ds_config.push_back(make_tuple(
			unit * (1 << i) * (i == k ? 4 : 1), 
			2, 
			i == k ? 4 : 1, 
			1 << (i-1)
		));
	return ds_config;
}

vector<tuple<int, int, int, int> > stair_config_separate(int memory, int k) {
	int sum = 0;
	for (int i = 0; i <= k; ++i) 
		sum += (1 << i); // * (i == k ? 4 : 1);

	double unit = (double) memory / sum;
	vector<tuple<int, int, int, int> > ds_config = 
		{ make_tuple(unit, 1, 1, 1) };

	for (int i = 1; i <= k; ++i)
		ds_config.push_back(make_tuple(
			unit * (1 << i), // * (i == k ? 4 : 1), 
			1 << i, 
			1, // i == k ? 4 : 1, 
			1
		));
	return ds_config;
}

stair_cu* build_scu(int memory, int level = 5) {
	return new stair_cu(stair_config(memory, level));
}

stair_bf* build_sbf(int memory, int level = 5) {
	return new stair_bf(stair_config(memory, level));
}

persistent_bf* build_pbf(int memory) {
	return new persistent_bf(cfg.win_num, memory, 2);
}

pbf1* build_pbf1(int memory) {
	return new pbf1(cfg.win_num, memory, 2);
}

pbf2* build_pbf2(int memory) {
	return new pbf2(cfg.win_num, memory, 2);
}

stair_cm* build_scm(int memory, int level = 5) {
	return new stair_cm(stair_config(memory, level));
}

stair_da* build_sda(int memory, int level = 5) {
	return new stair_da(stair_config_separate(memory, level));
}

stair_hll* build_shll(int memory, int level = 5) {
	return new stair_hll(stair_config_separate(memory, level));
}

stair_tower* build_stower(int memory, int level = 5) {
	return new stair_tower(stair_config(memory, level));
}

stair_elastic* build_selastic(int memory, int level = 5) {
	return new stair_elastic(stair_config_separate(memory, level));
}

item_aggregation_da* build_hda(int memory) {
	return new item_aggregation_da(memory, 2, cfg.ds_win_num);
}

item_aggregation_hll* build_hhll(int memory) {
	return new item_aggregation_hll(memory, 2, cfg.ds_win_num);
}

item_aggregation_tower* build_htower(int memory) {
	return new item_aggregation_tower(memory, 2, cfg.ds_win_num);
}

item_aggregation_elastic* build_helastic(int memory) {
	return new item_aggregation_elastic(memory, 2, cfg.ds_win_num);
}

item_aggregation_bf* build_iabf(int memory) {
	return new item_aggregation_bf(memory, 2, cfg.ds_win_num);
}

time_aggregation_bf* build_tabf(int memory, int lv_num) {
	return new time_aggregation_bf(memory, 2, lv_num);
}

ada_cm* build_adacm(int memory) {
	return new ada_cm(memory, 2);
}

ada_da* build_adada(int memory) {
	return new ada_da(memory, 2);
}

ada_elastic* build_adaelastic(int memory) {
	return new ada_elastic(memory, 2);
}

ada_tower* build_adatower(int memory) {
	return new ada_tower(memory, 2);
}

item_aggregation_cm* build_iacm(int memory) {
	return new item_aggregation_cm(memory, 2, cfg.ds_win_num);
}

pair<elem_t, int>* map_to_topk(std::unordered_map<elem_t, int> &map, int k = TOP_K){
	pair<elem_t, int>* result = new pair<elem_t, int>[k];
	vector<pair<elem_t, int>> arr;
	for (unordered_map<elem_t, int>::iterator it = map.begin(); it != map.end(); it++){
		arr.push_back(make_pair(it->first, it->second));
	}
	sort(arr.begin(), arr.end(), sortBySecondDesc);
	for (int i = 0; i < k; i++)
		if (i >= arr.size())
			result[i] = make_pair((elem_t)0, (int)-1);
		else 
			result[i] = arr[i];
	return result;
}
pair<elem_t, int>* topklist_to_topk(pair<elem_t, int>** list_begin, pair<elem_t, int>** list_end, int k = TOP_K){
	pair<elem_t, int>* result = new pair<elem_t, int>[k];
	vector<pair<elem_t, int>> arr;
	unordered_map<elem_t, int> freq, times;
	for (pair<elem_t, int>** listptr = list_begin; listptr != list_end; listptr++){
		pair<elem_t, int>* list = *listptr;
		for (int i = 0; i < k; i++){
			if (list[i].second == -1) continue;
			freq[list[i].first];
			times[list[i].first];
			freq[list[i].first] += list[i].second;
			times[list[i].first]++;
		}
	}
	for (unordered_map<elem_t, int>::iterator it = freq.begin(); it != freq.end(); it++){
		arr.push_back(make_pair(it->first, int(it->second / times[it->first])));
	}
	sort(arr.begin(), arr.end(), sortBySecondDesc);
	for (int i = 0; i < k; i++)
		if (i >= arr.size())
			result[i] = make_pair((elem_t)0, (int)-1);
		else 
			result[i] = arr[i];
	return result;
}

template<class Sketch>
void bf_test_fpr(Sketch *sketch, double *fpr) {	
	build_sketch(sketch);
	int start = cfg.win_num - cfg.ds_win_num + 1;
	for (int i = start; i <= cfg.win_num; ++i) {
		int fp = 0, tot = 0;
		for (int k = 0; k < elem_cnt; ++k) {
			if (elems[k].cnt[i] - elems[k].cnt[i-1] == 0) {
				tot++;
				if (sketch->query(i, elems[k].e)) fp++;
			} else {
				assert(sketch->query(i, elems[k].e));
			}
		}
		fpr[i - start + 1] = (double) fp / tot;
	}
}

template<class Sketch>
void bf_test_multi_fpr(Sketch *sketch, double *fpr) {	
	build_sketch(sketch);
	int *tot = new int[cfg.ds_win_num + 1];
	int *mask = new int[elem_cnt];
	int start = cfg.win_num - cfg.ds_win_num;

	for (int i = 1; i <= cfg.ds_win_num; ++i) fpr[i] = tot[i] = 0;
	for (int k = 0; k < elem_cnt; ++k) {
		mask[k] = 0;
		for (int i = 1; i <= cfg.ds_win_num; ++i)
			if (sketch->query(start + i, elems[k].e)) mask[k] |= 1 << i;
	}
	for (int l = 1; l <= cfg.ds_win_num; ++l) {
		double wt = 0; int _mask = 0;
		for (int r = l; r <= cfg.ds_win_num; ++r) {
			wt += 1.0 / (cfg.ds_win_num - r + 1); _mask += 1 << r;
			for (int k = 0; k < elem_cnt; ++k)
				if (elems[k].cnt[start + r] == elems[k].cnt[start + l - 1]) {
					tot[r - l + 1]++;
					if (mask[k] & _mask) fpr[r - l + 1] += wt;
				}
		}
	}
	for (int i = 1; i <= cfg.ds_win_num; ++i) fpr[i] /= tot[i];

	delete[] mask;
	delete[] tot;
}

void topk_test_stability_f1(framework *sketch, double *f1){
	for (int i = 1; i <= cfg.win_num; ++i) {
		if (sketch->add_delta_implemented()) {
			for (int k = 0; k < elem_cnt; ++k)
				if (elems[k].cnt[i] - elems[k].cnt[i-1] > 0)
					sketch->add(i, elems[k].e, elems[k].cnt[i]);
		} else {
			for (elem_t e : win_data[i]) sketch->add(i, e);
		}

		pair<elem_t, int> **answer_begin, **answer_end;
		answer_begin = sketch->query_topk(answer_end, i); 
		pair<elem_t, int>* ground_truth = map_to_topk(win_set[i]);
		pair<elem_t, int>* predict_result = topklist_to_topk(answer_begin, answer_end);
		f1[i] = calc_topk_F1(ground_truth, predict_result);
		for (pair<elem_t, int> **it = answer_begin; it != answer_end; it++)
			delete[] *it;
		delete[] answer_begin;
		delete[] ground_truth;
		delete[] predict_result;
	}
}

void topk_test_stability_are(framework *sketch, double *are){
	for (int i = 1; i <= cfg.win_num; ++i) {
		if (sketch->add_delta_implemented()) {
			for (int k = 0; k < elem_cnt; ++k)
				if (elems[k].cnt[i] - elems[k].cnt[i-1] > 0)
					sketch->add(i, elems[k].e, elems[k].cnt[i]);
		} else {
			for (elem_t e : win_data[i]) sketch->add(i, e);
		}

		pair<elem_t, int> **answer_begin, **answer_end;
		answer_begin = sketch->query_topk(answer_end, i); 
		pair<elem_t, int>* ground_truth = map_to_topk(win_set[i]);
		pair<elem_t, int>* predict_result = topklist_to_topk(answer_begin, answer_end);
		are[i] = calc_topk_ARE(ground_truth, predict_result);
		for (pair<elem_t, int> **it = answer_begin; it != answer_end; it++)
			delete[] *it;
		delete[] answer_begin;
		delete[] ground_truth;
		delete[] predict_result;
	}
}

void cardinal_test_stability_are(framework *sketch, double *are){
	for (int i = 1; i <= cfg.win_num; ++i) {
		if (sketch->add_delta_implemented()) {
			for (int k = 0; k < elem_cnt; ++k)
				if (elems[k].cnt[i] - elems[k].cnt[i-1] > 0)
					sketch->add(i, elems[k].e, elems[k].cnt[i]);
		} else {
			for (elem_t e : win_data[i]) sketch->add(i, e);
		}

		int ground_truth = win_set[i].size();
		int predict_result = sketch->query(i); 
		are[i] = fabs(ground_truth - predict_result) / (double)ground_truth;
	}
}

void cnt_test_stability_are(framework *sketch, double *are){
	for (int i = 1; i <= cfg.win_num; ++i) {
		if (sketch->add_delta_implemented()) {
			for (int k = 0; k < elem_cnt; ++k)
				if (elems[k].cnt[i] - elems[k].cnt[i-1] > 0)
					sketch->add(i, elems[k].e, elems[k].cnt[i] - elems[k].cnt[i-1]);
		} else {
			for (elem_t e : win_data[i]) sketch->add(i, e);
		}

		int tot = 0; are[i] = 0;	
		for (auto pr : win_set[i]) {
			int real = pr.second, ans = sketch->query(i, pr.first);
			are[i] += 1.0 * fabs(real - ans) / real;
			tot++;
		}
		are[i] /= tot;
	}
}

void cnt_test_stability_aae(framework *sketch, double *aae){
	for (int i = 1; i <= cfg.win_num; ++i) {
		if (sketch->add_delta_implemented()) {
			for (int k = 0; k < elem_cnt; ++k)
				if (elems[k].cnt[i] - elems[k].cnt[i-1] > 0)
					sketch->add(i, elems[k].e, elems[k].cnt[i] - elems[k].cnt[i-1]);
		} else {
			for (elem_t e : win_data[i]) sketch->add(i, e);
		}

		int tot = 0; aae[i] = 0;	
		for (auto pr : win_set[i]) {
			int real = pr.second, ans = sketch->query(i, pr.first);
			aae[i] += 1.0 * fabs(real - ans);
			tot++;
		}
		aae[i] /= tot;
	}
}

template<class Sketch>
void bf_test_stability(Sketch *sketch, double *fpr) {
	for (int i = 1; i <= cfg.win_num; ++i) {
		if (sketch->add_delta_implemented()) {
			for (int k = 0; k < elem_cnt; ++k)
				if (elems[k].cnt[i] - elems[k].cnt[i-1] > 0)
					sketch->add(i, elems[k].e, elems[k].cnt[i]);
		} else {
			for (elem_t e : win_data[i]) sketch->add(i, e);
		}

		int fp = 0, tot = 0;
		for (int k = 0; k < elem_cnt; ++k) {
			if (elems[k].cnt[i] - elems[k].cnt[i-1] == 0) {
				tot++;
				if (sketch->query(i, elems[k].e)) fp++;
			} else {
				assert(sketch->query(i, elems[k].e));
			}
		}
		fpr[i] = (double) fp / tot;
	}
	fprintf(stderr, "%d/%d\n", sketch->memory(), cfg.memory);
}


void cnt_test_are(framework *sketch, double *are) {
	build_sketch(sketch);
	fprintf(stderr, "Memory %d/%d\n", sketch->memory(), cfg.memory);
	int start = cfg.win_num - cfg.ds_win_num + 1;
	for (int i = start; i <= cfg.win_num; ++i) {
		int tot = 0; are[i - start + 1] = 0;	
		for (auto pr : win_set[i]) {
			int real = pr.second, ans = sketch->query(i, pr.first);
			are[i - start + 1] += 1.0 * fabs(real - ans) / real;
			tot++;
		}
		are[i - start + 1] /= tot;
	}
}

void topk_test_f1(framework *sketch, double *f1) {
	build_sketch(sketch);
	fprintf(stderr, "Memory %d/%d\n", sketch->memory(), cfg.memory);
	int start = cfg.win_num - cfg.ds_win_num + 1;
	for (int i = start; i <= cfg.win_num; ++i) {
		// compare topk
		pair<elem_t, int> **answer_begin, **answer_end;
		answer_begin = sketch->query_topk(answer_end, i); 
		pair<elem_t, int>* ground_truth = map_to_topk(win_set[i]);
		pair<elem_t, int>* predict_result = topklist_to_topk(answer_begin, answer_end);
		f1[i - start + 1] = calc_topk_F1(ground_truth, predict_result);
		for (pair<elem_t, int> **it = answer_begin; it != answer_end; it++)
			delete[] *it;
		delete[] answer_begin;
		delete[] ground_truth;
		delete[] predict_result;
	}
}

void topk_test_are(framework *sketch, double *are) {
	build_sketch(sketch);
	fprintf(stderr, "Memory %d/%d\n", sketch->memory(), cfg.memory);
	int start = cfg.win_num - cfg.ds_win_num + 1;
	for (int i = start; i <= cfg.win_num; ++i) {
		// compare topk
		pair<elem_t, int> **answer_begin, **answer_end;
		answer_begin = sketch->query_topk(answer_end, i); 
		pair<elem_t, int>* ground_truth = map_to_topk(win_set[i]);
		pair<elem_t, int>* predict_result = topklist_to_topk(answer_begin, answer_end);
		are[i - start + 1] = calc_topk_ARE(ground_truth, predict_result);
		// see different win data
		fprintf(stderr, "%d win %d:%d, %d:%d\n", i, 
			ground_truth[0].second, predict_result[0].second,
			ground_truth[TOP_K - 1].second, predict_result[TOP_K - 1].second);
		//
		for (pair<elem_t, int> **it = answer_begin; it != answer_end; it++)
			delete[] *it;
		delete[] answer_begin;
		delete[] ground_truth;
		delete[] predict_result;
	}
}

void topk_test_aae(framework *sketch, double *aee) {
	build_sketch(sketch);
	fprintf(stderr, "Memory %d/%d\n", sketch->memory(), cfg.memory);
	int start = cfg.win_num - cfg.ds_win_num + 1;
	for (int i = start; i <= cfg.win_num; ++i) {
		// compare topk
		pair<elem_t, int> **answer_begin, **answer_end;
		answer_begin = sketch->query_topk(answer_end, i); 
		pair<elem_t, int>* ground_truth = map_to_topk(win_set[i]);
		pair<elem_t, int>* predict_result = topklist_to_topk(answer_begin, answer_end);
		aee[i - start + 1] = calc_topk_AEE(ground_truth, predict_result);
		for (pair<elem_t, int> **it = answer_begin; it != answer_end; it++)
			delete[] *it;
		delete[] answer_begin;
		delete[] ground_truth;
		delete[] predict_result;
	}
}

void cardinal_test_are(framework *sketch, double *are) {
	build_sketch(sketch);
	fprintf(stderr, "Memory %d/%d\n", sketch->memory(), cfg.memory);
	int start = cfg.win_num - cfg.ds_win_num + 1;
	for (int i = start; i <= cfg.win_num; ++i) {
		// compare topk
		int ground_truth = win_set[i].size();
		int predict_result = sketch->query(i); 
		are[i - start + 1] = fabs(ground_truth - predict_result) / (double)ground_truth;
	}
}

template<class Sketch>
void cnt_test_multi_are(Sketch *sketch, double *are) {	
	build_sketch(sketch);
	int **sum = new int*[elem_cnt];
	int *tot = new int[cfg.ds_win_num + 1];
	int start = cfg.win_num - cfg.ds_win_num;
	for (int k = 0; k < elem_cnt; ++k) {
		sum[k] = new int[cfg.ds_win_num + 1];
		sum[k][0] = 0;
		for (int i = 1; i <= cfg.ds_win_num; ++i)
			sum[k][i] = sum[k][i-1] + sketch->query(start + i, elems[k].e);
	}

	for (int i = 1; i <= cfg.ds_win_num; ++i) are[i] = tot[i] = 0;
	for (int l = 1; l <= cfg.ds_win_num; ++l) {
		double wt = 0;
		for (int r = l; r <= cfg.ds_win_num; ++r) {
			wt += 1.0 / (cfg.ds_win_num - r + 1);
			for (int k = 0; k < elem_cnt; ++k)
				if (elems[k].cnt[start + r] > elems[k].cnt[start + l - 1]) {
					tot[r - l + 1]++;
					int real = elems[k].cnt[start + r] - elems[k].cnt[start + l - 1], ans = sum[k][r] - sum[k][l - 1];
					are[r - l + 1] += wt * fabs(real - ans) / real;
				}
		}
	}
	for (int i = 1; i <= cfg.win_num; ++i) are[i] /= tot[i];

	for (int k = 0; k < elem_cnt; ++k) delete[] sum[k];
	delete[] sum;
	delete[] tot;
}

template<class Sketch>
void cnt_test_multi_aae(Sketch *sketch, double *aae) {	
	build_sketch(sketch);
	int **sum = new int*[elem_cnt];
	int *tot = new int[cfg.ds_win_num + 1];
	int start = cfg.win_num - cfg.ds_win_num;
	for (int k = 0; k < elem_cnt; ++k) {
		sum[k] = new int[cfg.ds_win_num + 1];
		sum[k][0] = 0;
		for (int i = 1; i <= cfg.ds_win_num; ++i)
			sum[k][i] = sum[k][i-1] + sketch->query(start + i, elems[k].e);
	}

	for (int i = 1; i <= cfg.ds_win_num; ++i) aae[i] = tot[i] = 0;
	for (int l = 1; l <= cfg.ds_win_num; ++l) {
		double wt = 0;
		for (int r = l; r <= cfg.ds_win_num; ++r) {
			wt += 1.0 / (cfg.ds_win_num - r + 1);
			for (int k = 0; k < elem_cnt; ++k)
				if (elems[k].cnt[start + r] > elems[k].cnt[start + l - 1]) {
					tot[r - l + 1]++;
					int real = elems[k].cnt[start + r] - elems[k].cnt[start + l - 1], ans = sum[k][r] - sum[k][l - 1];
					aae[r - l + 1] += wt * fabs(real - ans);
				}
		}
	}
	for (int i = 1; i <= cfg.win_num; ++i) aae[i] /= tot[i];

	for (int k = 0; k < elem_cnt; ++k) delete[] sum[k];
	delete[] sum;
	delete[] tot;
}

void cnt_test_aae(framework *sketch, double *aae) {
	build_sketch(sketch);
	int start = cfg.win_num - cfg.ds_win_num + 1;
	for (int i = start; i <= cfg.win_num; ++i) {
		int tot = 0; aae[i - start + 1] = 0;	
		for (int k = 0; k < elem_cnt; ++k) {
			if (elems[k].cnt[i] - elems[k].cnt[i-1] > 0) {
				int real = elems[k].cnt[i] - elems[k].cnt[i-1], ans = sketch->query(i, elems[k].e);
				aae[i - start + 1] += fabs(real - ans);
				tot++;
			}
		}
		// fprintf(stderr, "\"%d\" ", tot);
		// if (tot != 0) 
		aae[i - start + 1] /= tot;
	}
}

template<class Sketch>
void cnt_test_stability(Sketch *sketch, double *are) {
	for (int i = 1; i <= cfg.win_num; ++i) {
		if (sketch->add_delta_implemented()) {
			for (int k = 0; k < elem_cnt; ++k)
				if (elems[k].cnt[i] - elems[k].cnt[i-1] > 0)
					sketch->add(i, elems[k].e, elems[k].cnt[i] - elems[k].cnt[i-1]);
		} else {
			for (elem_t e : win_data[i]) sketch->add(i, e);
		}
		int tot = 0; are[i] = 0;	
		for (int k = 0; k < elem_cnt; ++k) {
			if (elems[k].cnt[i] - elems[k].cnt[i-1] > 0) {
				int real = elems[k].cnt[i] - elems[k].cnt[i-1], ans = sketch->query(i, elems[k].e);
				are[i] += 1.0 * fabs(real - ans) / real;
				tot++;
			}
		}
		are[i] /= tot;
	}
}

double weighted_score(double *score) {
	double ret = 0;
	for (int i = 1; i <= cfg.ds_win_num; ++i) {
		ret += score[i] / (cfg.ds_win_num - i + 1);
		fprintf(stderr, "%.4f ", score[i]);
	}
	fprintf(stderr, " %.4f\n", ret);
	return ret;
}

double weighted_F1_score(double *F1_score) {
	double ret = 0;
	for (int i = 1; i <= cfg.ds_win_num; ++i) {
		ret += (1 - F1_score[i]) / (cfg.ds_win_num - i + 1);
		fprintf(stderr, "%.4f ", F1_score[i]);
	}
	fprintf(stderr, " %.4f\n", ret);
	return ret;
}

template<class Sketch>
double bf_test_wfpr(Sketch *s) {
	double* fpr = new double[cfg.ds_win_num + 1];
	bf_test_fpr(s, fpr);
	return weighted_score(fpr);
}

double cnt_test_ware(framework *s) {
	double* are = new double[cfg.ds_win_num + 1];
	cnt_test_are(s, are);
	return weighted_score(are);
}
// template<class Sketch>
// double topk_test_wf1(Sketch *s) {
// 	double* are = new double[cfg.ds_win_num + 1];
// 	topk_test_f1(s, are);
// 	return weighted_f1_score(are);
// }

double topk_test_wf1(framework *s) {
	double* f1 = new double[cfg.ds_win_num + 1];
	topk_test_f1(s, f1);
	double result = weighted_F1_score(f1);
	delete[] f1;
	return result;
}

double topk_test_ware(framework *s) {
	double* are = new double[cfg.ds_win_num + 1];
	topk_test_are(s, are);
	double result = weighted_score(are);
	delete[] are;
	return result;
}

double topk_test_waae(framework *s) {
	double* aae = new double[cfg.ds_win_num + 1];
	topk_test_aae(s, aae);
	double result = weighted_score(aae);
	delete[] aae;
	return result;
}

double cardinal_test_ware(framework *s) {
	double* are = new double[cfg.ds_win_num + 1];
	cardinal_test_are(s, are);
	double return_value = weighted_score(are);
	delete[] are;
	return return_value;
}

double cnt_test_waae(framework *s) {
	double* aae = new double[cfg.ds_win_num + 1];
	cnt_test_aae(s, aae);
	return weighted_score(aae);
}

template<class Sketch>
double bf_test_win_num_wfpr(Sketch *sketch) {	
	double *fpr = new double[cfg.ds_win_num + 1];
	int start = cfg.win_num - cfg.ds_win_num;
	for (int i = 1; i <= cfg.win_num; ++i)
		for (elem_t e : win_data[i])
			sketch->add(i, e);
	for (int i = 1; i <= cfg.ds_win_num; ++i) {
		int fp = 0, tot = 0;
		for (elem_t e : elem_set)
			if (win_set[start + i].count(e) == 0) {
				++tot;
				if (sketch->query(start + i, e)) ++fp;
			}
		fpr[i] = 1.0 * fp / tot;
	}
	return weighted_score(fpr);
}

template<class Sketch>
double cnt_test_win_num_ware(Sketch *sketch) {
	double *are = new double[cfg.ds_win_num + 1];
	int start = cfg.win_num - cfg.ds_win_num;
	for (int i = 1; i <= cfg.win_num; ++i)
		for (elem_t e : win_data[i])
			sketch->add(i, e);
	for (int i = 1; i <= cfg.ds_win_num; ++i) {
		are[i] = 0;
		int tot = 0;
		for (auto pr : win_set[start + i]) {
			int real = pr.second, ans = sketch->query(start + i, pr.first);
			are[i] += 1.0 * fabs(real - ans) / real;
			++tot;
		}
		are[i] /= tot;
	}
	return weighted_score(are);
}

template<class Sketch>
double cnt_test_win_num_waae(Sketch *sketch) {
	double *aae = new double[cfg.ds_win_num + 1];
	int start = cfg.win_num - cfg.ds_win_num;
	for (int i = 1; i <= cfg.win_num; ++i)
		for (elem_t e : win_data[i])
			sketch->add(i, e);
	for (int i = 1; i <= cfg.ds_win_num; ++i) {
		aae[i] = 0;
		int tot = 0;
		for (auto pr : win_set[start + i]) {
			int real = pr.second, ans = sketch->query(start + i, pr.first);
			aae[i] += 1.0 * fabs(real - ans);
			++tot;
		}
		aae[i] /= tot;
	}
	return weighted_score(aae);
}

template<class Sketch>
void bf_test_qcnt(Sketch *sketch, double *aqcnt) {
	build_sketch(sketch);
	int start = cfg.win_num - cfg.ds_win_num;
	int *tot = new int[cfg.ds_win_num + 1];
	long long *cnt = new long long[cfg.ds_win_num + 1];
	for (int i = 1; i <= cfg.ds_win_num; ++i) cnt[i] = tot[i] = 0;
	long long last = 0;

	for (int l = 1; l <= cfg.ds_win_num; ++l) {
		for (int r = l; r <= cfg.ds_win_num; ++r) {
			for (int k = 0; k < elem_cnt; ++k)
				if (elems[k].cnt[start + r] == elems[k].cnt[start + l - 1]) {
					tot[r - l + 1]++;
					long long last = sketch->qcnt();
					sketch->query_multiple_windows(l, r, elems[k].e);
					cnt[r - l + 1] += sketch->qcnt() - last;
				}
		}
	}
	for (int i = 1; i <= cfg.ds_win_num; ++i)
		aqcnt[i] = 1.0 * cnt[i] / tot[i];

	delete[] cnt;
	delete[] tot;
}

// template<class Sketch>
void cnt_test_qcnt(/*Sketch*/framework *sketch, double *aqcnt) {	
	build_sketch(sketch);
	int *tot = new int[cfg.ds_win_num + 1];
	int start = cfg.win_num - cfg.ds_win_num;
	long long *cnt = new long long[cfg.ds_win_num + 1];
	for (int i = 1; i <= cfg.ds_win_num; ++i) cnt[i] = tot[i] = 0;
	long long last = 0;

	for (int l = 1; l <= cfg.ds_win_num; ++l) {
		fprintf(stderr, "Testing l = %d / %d\n", l, cfg.ds_win_num);
		for (int r = l; r <= cfg.ds_win_num; ++r) {
			for (int k = 0; k < elem_cnt; ++k)
				if (elems[k].cnt[start + r] > elems[k].cnt[start + l - 1]) {
					tot[r - l + 1]++;
					long long last = sketch->qcnt();
					sketch->query_multiple_windows(l, r, elems[k].e);
					cnt[r - l + 1] += sketch->qcnt() - last;
				}
		}
	}
	for (int i = 1; i <= cfg.ds_win_num; ++i)
		aqcnt[i] = 1.0 * cnt[i] / tot[i];

	delete[] cnt;
	delete[] tot;
}

void cnt_test_qcnt_se(framework *sketch, double *aqcnt) {	
	build_sketch(sketch);
	int *tot = new int[cfg.ds_win_num + 1];
	int start = cfg.win_num - cfg.ds_win_num;
	long long *cnt = new long long[cfg.ds_win_num + 1];
	for (int i = 1; i <= cfg.ds_win_num; ++i) cnt[i] = tot[i] = 0;
	long long last = 0;

	for (int l = 1; l <= cfg.ds_win_num; ++l) {
		for (int r = l; r <= cfg.ds_win_num; ++r) {
			tot[r - l + 1]++;
			long long last = sketch->qcnt();
			sketch->query_multiple_windows(l, r, -1); // same qcnt for different element
			cnt[r - l + 1] += sketch->qcnt() - last;
		}
	}
	for (int i = 1; i <= cfg.ds_win_num; ++i)
		aqcnt[i] = 1.0 * cnt[i] / tot[i];

	delete[] cnt;
	delete[] tot;
}

void topk_test_qcnt(framework *sketch, double *aqcnt) {	
	build_sketch(sketch);
	int *tot = new int[cfg.ds_win_num + 1];
	int start = cfg.win_num - cfg.ds_win_num;
	long long *cnt = new long long[cfg.ds_win_num + 1];
	for (int i = 1; i <= cfg.ds_win_num; ++i) cnt[i] = tot[i] = 0;
	long long last = 0;

	for (int l = 1; l <= cfg.ds_win_num; ++l) {
		for (int r = l; r <= cfg.ds_win_num; ++r) {
			tot[r - l + 1]++;
			long long last = sketch->qcnt();
			pair<elem_t, int> **answer_begin, **answer_end;
			answer_begin = sketch->query_multiple_windows_topk(answer_end, l, r);
			cnt[r - l + 1] += sketch->qcnt() - last;
		}
	}
	for (int i = 1; i <= cfg.ds_win_num; ++i)
		aqcnt[i] = 1.0 * cnt[i] / tot[i];

	delete[] cnt;
	delete[] tot;
}

#include <chrono>
using namespace std::chrono;
using hrc = std::chrono::high_resolution_clock;
double test_speed(framework *sketch) {	
	int cnt = 0;
	hrc::time_point t1 = hrc::now();
	for (int i = 1; i <= cfg.ds_win_num; ++i)
		for (elem_t e : win_data[i])
			sketch->add(i, e), cnt++;
	hrc::time_point t2 = hrc::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	return cnt / time_span.count();
}

template<class Sketch>
double test_speed_old(Sketch *sketch) {	
	int cnt = 0;
	hrc::time_point t1 = hrc::now();
	for (int i = 1; i <= cfg.ds_win_num; ++i)
		for (elem_t e : win_data[i])
			sketch->add(i, e), cnt++;
	hrc::time_point t2 = hrc::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	return cnt / time_span.count();
}



void ada_test_diff_window_are(framework *sketch, double *are){
	for (int i = 1; i <= cfg.win_num; ++i) {
		if (sketch->add_delta_implemented()) {
			for (int k = 0; k < elem_cnt; ++k)
				if (elems[k].cnt[i] - elems[k].cnt[i-1] > 0)
					sketch->add(i, elems[k].e, elems[k].cnt[i]);
		} else {
			for (elem_t e : win_data[i]) sketch->add(i, e);
		}

		for (int j = 1; j <= i; j++){
			int tot = 0; are[j] = 0;
			for (auto pr : win_set[j]) {
				int real = pr.second, ans = sketch->query(j, pr.first);
				are[j] += 1.0 * fabs(real - ans) / real;
				tot++;
			}
			are[j] /= tot;
		}
		for (int j = 1; j <= i; j++) printf("%.6f,", are[j]);
		printf("\n");
	}
}


double ada_test_diff_window_f1(framework *sketch){
	double f1[40];
	for (int i = 1; i <= cfg.win_num; ++i) {
		if (sketch->add_delta_implemented()) {
			for (int k = 0; k < elem_cnt; ++k)
				if (elems[k].cnt[i] - elems[k].cnt[i-1] > 0)
					sketch->add(i, elems[k].e, elems[k].cnt[i]);
		} else {
			for (elem_t e : win_data[i]) sketch->add(i, e);
		}

		sketch->name();

		for (int j = 1; j <= i; j++){
			pair<elem_t, int> **answer_begin, **answer_end;
			answer_begin = sketch->query_topk(answer_end, j); 
			pair<elem_t, int>* ground_truth = map_to_topk(win_set[j]);
			pair<elem_t, int>* predict_result = topklist_to_topk(answer_begin, answer_end);
			f1[j] = calc_topk_F1(ground_truth, predict_result);
			for (pair<elem_t, int> **it = answer_begin; it != answer_end; it++)
				delete[] *it;
			delete[] answer_begin;
			delete[] ground_truth;
			delete[] predict_result;
		}
		for (int j = 1; j <= i; j++) printf("%.6f,", f1[j]);
		printf("\n");
	}
	return 0;
}