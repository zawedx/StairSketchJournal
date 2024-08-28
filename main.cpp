#define DEBUG_FLAG

#include "impl_all.h"
#include "test.cpp"
#include <algorithm>
#include <functional>


#define CAIDA_DATA_RANGE 960
#define SELECTED_DATASET "webpage"
#define TEST_REPEAT_TIME 1
// #define TEST_DA
#define TEST_ELASTIC
// #define TEST_TOWER
// #define TEST_CMCU
// #define TEST_HLL
// #define TEST_BF


void dump_topk_total_freq(unordered_map<elem_t, int> &data, int &total, int &thres){
	vector<int> all;
	all.clear();
	for (auto it : data){
		all.push_back(it.second);
	}
	sort(all.begin(), all.end(), greater<int>());
	total = 0;
	for (int i = 0; i < TOP_K; i++){
		total += all[i];
	}
	thres = all[TOP_K];
}

void initialize(bool save = false, double win_time = cfg.win_time) {
	for (int i = 1; i <= cfg.win_num; ++i)
		win_data[i].clear(), win_set[i].clear();

	file_reader* fr = new file_reader(cfg);
	// HashMap* hmap = new HashMap;
	elem_t e; double ts, ts_begin;
	fr->read(e, ts); ts_begin = ts;
	int count_read = 1;
	unordered_map<elem_t, int> count_win32;
	int count_win32_total = 0;
	for (int i = 1; i <= cfg.win_num; ) {
		// hmap->add(i, e);
		if (save) {
			win_data[i].push_back(e);
			win_set[i][e]++;
			elem_set.insert(e);
		}
		if (!fr->read(e, ts)) break;
		if (i == 32) count_win32[e] = 1;
		if (i == 32) count_win32_total ++;
		count_read++;
	#ifdef DEBUG_FLAG
		if (count_read % 4000000 == 0) 
			fprintf(stderr, "\"i=%d, ts-tsbegin=%.4lf\"\n", i, ts - ts_begin);
	#endif
		if (ts - ts_begin > i * win_time){
			// fprintf(stderr, "\"diff = %.8f - %.8f\" ", ts, ts_begin);
			i++;
		}
	}
	for (int i = 1; i <= cfg.win_num; i++) {
		int total = 0, thres = 0;
		dump_topk_total_freq(win_set[i], total, thres);
		fprintf(stderr, "\"win %d item = %d total, %d unique, Ksum %d, min %d\"\n",
			i, win_data[i].size(), win_set[i].size(), total, thres);
	}
	fprintf(stderr, "\"win32=%d\"\n", (int)count_win32.size());
	fprintf(stderr, "\"win32total=%d\"\n", count_win32_total);
	fprintf(stderr, "\"total read=%d\"\n", count_read);
	if (elems != nullptr) delete elems;
	// elem_cnt = hmap->all_elements(elems);
	// fprintf(stderr, "\"distinct value=%d\"\n", elem_cnt);
	delete fr;
	// delete hmap;

	fprintf(stderr, "initialization complete\n");
}

void initialize_win_num_test() {
	elem_set.clear();
	for (int i = 1; i <= cfg.win_num; ++i)
		win_data[i].clear(), win_set[i].clear();

	file_reader* fr = new file_reader(cfg);
	elem_t e; double ts, ts_begin;
	fr->read(e, ts); ts_begin = ts;	
	
	for (int i = 1; i <= cfg.win_num; ) {
		win_data[i].push_back(e);
		win_set[i][e]++;
		elem_set.insert(e);
		if (!fr->read(e, ts)) break;
		if (ts - ts_begin > i * cfg.win_time) 
			fprintf(stderr, "\"i=%d\" ", i);
		if (ts - ts_begin > i * cfg.win_time)
			i++;
	}
	delete fr;
}

void output_line(const char* name, double *arr, FILE *fp, int n = cfg.ds_win_num) {
	fprintf(fp, "%s,", name);
	for (int i = 1; i <= n; ++i)
		fprintf(fp, "%.8f%c", arr[i], i == n ? '\n' : ',');
	fflush(fp);
}

void output_line(string name, double *arr, FILE *fp, int n = cfg.ds_win_num) {
	fprintf(fp, "%s,", name.c_str());
	for (int i = 1; i <= n; ++i)
		fprintf(fp, "%.8f%c", arr[i], i == n ? '\n' : ',');
	fflush(fp);
}

int MB(int n) { return n * 1024 * 1024; }
int KB(int n) { return n * 1024; }

template<class sketch> void test1(const char* name, sketch *sk, double *fpr, FILE *fp) {
	bf_test_fpr(sk, fpr);
	output_line(name, fpr, fp);
}

void figure1(const char* file_name) {
	initialize();
	FILE *fp = fopen(file_name, "w");
	double *fpr = new double[cfg.win_num + 1];

	fprintf(fp, "window id,");
	for (int i = 1; i <= cfg.win_num; ++i)
		fprintf(fp, "%d,", i);
	fprintf(fp, "\n");

	test1("sbf",  build_sbf(cfg.memory, 5),  fpr, fp);
	test1("pbf",  build_pbf(cfg.memory),  fpr, fp);
	test1("iabf", build_iabf(cfg.memory), fpr, fp);

	fclose(fp);
	delete[] fpr;
}

// void figure1_1() {
// 	cfg = config("CAIDA", 32, 32, 60, MB(10));
// 	figure1("figure1_1.csv");
// }

// void figure1_2() {// not appearing in the paper
// 	cfg = config("zipf", 8, 8, 60, MB(10));
// 	figure1("figure1_2.csv");
// }

// void figure1_3() {
// 	cfg = config("webpage", 32, 32, 60, MB(4));
// 	figure1("figure1_3.csv");
// }

void figure2(const char* file_name) {
	initialize();
	FILE *fp = fopen(file_name, "w");
	double *fpr = new double[cfg.win_num + 1];

	fprintf(fp, "window id,");
	for (int i = 1; i <= cfg.win_num; ++i)
		fprintf(fp, "%d,", i);
	fprintf(fp, "\n");

	bf_test_stability(build_sbf(cfg.memory), fpr);
	output_line("sbf", fpr, fp, cfg.win_num);

	bf_test_stability(build_pbf(cfg.memory), fpr);
	output_line("pbf", fpr, fp, cfg.win_num);
	
	bf_test_stability(build_iabf(cfg.memory), fpr);
	output_line("iabf", fpr, fp, cfg.win_num);

	fclose(fp);
}

// void figure2_1() {
// 	cfg = config("CAIDA", 16, 8, 60, MB(6));
// 	figure2("figure2_1.csv");
// }

// void figure2_2() {// not appearing in the paper
// 	cfg = config("zipf", 16, 8, 60, MB(5));
// 	figure2("figure2_2.csv");
// }

// void figure2_3() {
// 	cfg = config("webpage", 16, 8, 60, MB(2));
// 	figure2("figure2_3.csv");
// }

// template<class sketch> void test3(const char* name, sketch *sk, double *are, FILE *fp) {
// 	cnt_test_are(sk, are);
// 	output_line(name, are, fp);
// }

// void figure3(const char* file_name) {
// 	initialize(true);
// 	FILE *fp = fopen(file_name, "w");
// 	double *are = new double[cfg.win_num + 1];

// 	fprintf(fp, "window id,");
// 	for (int i = 1; i <= cfg.ds_win_num; ++i)
// 		fprintf(fp, "%d,", i);
// 	fprintf(fp, "\n");

// 	test3("scm",   build_scm(cfg.memory),   are, fp);
// 	test3("scu",   build_scu(cfg.memory),   are, fp);
// 	test3("adacm", build_adacm(cfg.memory), are, fp);
// 	test3("iacm",  build_iacm(cfg.memory),  are, fp);

// 	fclose(fp);
// }

// void figure3_1() {
// 	cfg = config("CAIDA", 8, 8, 60, 80*1024*1024);
// 	figure3("figure3_1_.csv");
// }

// void figure3_2() {
// 	cfg = config("zipf", 8, 8, 60, 200*1024*1024);
// 	figure3("figure3_2.csv");
// }

// void figure3_3() {
// 	cfg = config("webpage", 8, 8, 60, 50*1024*1024);
// 	figure3("figure3_3.csv");
// }

void test4(const char* name, framework *sk, double *are, FILE *fp) {
	cnt_test_stability(sk, are);
	output_line(name, are, fp, cfg.win_num);
}

void figure4(const char* file_name) {
	// initialize(true);
	FILE *fp = fopen(file_name, "w");
	double *are = new double[cfg.win_num + 1];

	fprintf(fp, "window id,");
	for (int i = 1; i <= cfg.win_num; ++i)
		fprintf(fp, "%d,", i);
	fprintf(fp, "\n");

	test4("scm",   build_scm(cfg.memory),   are, fp);
	test4("scu",   build_scu(cfg.memory),   are, fp);
	test4("adacm", build_adacm(cfg.memory), are, fp);
	test4("iacm",  build_iacm(cfg.memory),  are, fp);

	fclose(fp);
}

// void figure4_1() {
// 	cfg = config("CAIDA", 16, 8, 60, MB(60));
// 	figure4("figure4_1.csv");
// }

// void figure4_2() {
// 	cfg = config("zipf", 16, 8, 60, MB(100));
// 	figure4("figure4_2.csv");
// }

void figure4_3() {
	cfg = config("webpage", 64, 64, 60, MB(60));
	figure4("figure4_3.csv");
}

void figure5(const char* file_name, int m_begin, int m_end, int m_step) {
	FILE *fp = fopen(file_name, "w");
	initialize();
	fprintf(fp, "Memory(MB),SBF,PBF,IABF\n");
	for (int mem = m_begin; mem <= m_end; mem += m_step) {
		cfg.memory = mem;
		fprintf(fp, "%d,%.8f,%.8f,%.8f\n", mem/1024/1024, bf_test_wfpr(build_sbf(mem, 5)),
			bf_test_wfpr(build_pbf(mem)), bf_test_wfpr(build_iabf(mem)));
		fflush(fp);
	}
	fclose(fp);
}

// void figure5_1() {
// 	cfg = config("CAIDA", 8, 8, 60, MB(2));
// 	figure5("figure5_1.csv", MB(2), MB(16), MB(2));
// }

// void figure5_2() {
// 	cfg = config("zipf", 8, 8, 60, MB(8));
// 	figure5("figure5_2.csv", MB(8), MB(32), MB(4));
// }

// void figure5_3() {
// 	cfg = config("webpage", 32, 32, 60, MB(2));
// 	figure5("figure5_3.csv", MB(2), MB(16), MB(2));
// }

// void figure6(const char* file_name, int m_begin, int m_end, int m_step) {
// 	FILE *fp = fopen(file_name, "w");
// 	initialize(true);
// 	fprintf(fp, "Memory(MB),SCM,SCU,Ada CM,IACM\n");
// 	for (int mem = m_begin; mem <= m_end; mem += m_step) {
// 		cfg.memory = mem;
// 		fprintf(fp, "%d,%.8f,%.8f,%.8f,%.8f\n", mem/1024/1024, 
// 			cnt_test_ware(build_scm(mem)), cnt_test_ware(build_scu(mem)),
// 			cnt_test_ware(build_adacm(mem)), cnt_test_ware(build_iacm(mem)));
// 		fflush(fp);
// 	}
// 	fclose(fp);
// }

// void figure6_1() {
// 	cfg = config("CAIDA", 32, 32, 60, MB(100));
// 	figure6("figure6_1.csv", MB(100), MB(180), MB(10));
// }

// void figure6_2() {
// 	cfg = config("zipf", 32, 32, 60, MB(160));
// 	figure6("figure6_2.csv", MB(160), MB(240), MB(10));
// }

// void figure6_3() {
// 	cfg = config("webpage", 32, 32, 60, MB(40));
// 	figure6("figure6_3.csv",MB(40), MB(80), MB(5));
// }

// void figure7(const char* file_name, int m_begin, int m_end, int m_step) {
// 	FILE *fp = fopen(file_name, "w");
// 	initialize(true);
// 	fprintf(fp, "Memory(MB),SCM,SCU,Ada CM,IACM\n");
// 	for (int mem = m_begin; mem <= m_end; mem += m_step) {
// 		cfg.memory = mem;
// 		fprintf(fp, "%d,%.8f,%.8f,%.8f,%.8f\n", mem/1024/1024, 
// 			cnt_test_waae(build_scm(mem)), cnt_test_waae(build_scu(mem)),
// 			cnt_test_waae(build_adacm(mem)), cnt_test_waae(build_iacm(mem)));
// 		fflush(fp);
// 	}
// 	fclose(fp);
// }

// void figure7_1() {
// 	cfg = config("CAIDA", 8, 8, 60, MB(100));
// 	figure7("figure7_1.csv", MB(100), MB(180), MB(10));
// }

// void figure7_2() {
// 	cfg = config("zipf", 8, 8, 60, MB(160));
// 	figure7("figure7_2.csv", MB(160), MB(240), MB(10));
// }

// void figure7_3() {
// 	cfg = config("webpage", 8, 8, 60, MB(40));
// 	figure7("figure7_3.csv", MB(40), MB(80), MB(5));
// }

void figure8_1(const char* file_name) {
	FILE *fp = fopen(file_name, "w");
	const int mem = MB(20);
	cfg = config("CAIDA", 8, 8, 60, mem);
	fprintf(fp, "Window Num,SBF,PBF,IABF\n");
	for (int k = 3; k <= 8; ++k) {
		int win = 1 << k;
		cfg.ds_win_num = win;
		cfg.win_num = win;
		cfg.win_time = 480.0 / cfg.win_num;
		initialize_win_num_test();
		fprintf(fp, "%d,%.8f,%.8f,%.8f\n", win, bf_test_win_num_wfpr(build_sbf(mem, k)), 
			bf_test_win_num_wfpr(build_pbf(mem)), bf_test_win_num_wfpr(build_iabf(mem)));
		fflush(fp);
	}
	fclose(fp);
}

// void figure8_2() {
// 	FILE *fp = fopen("figure8_2.csv", "w");
// 	const int mem = MB(300);
// 	cfg = config("CAIDA", 8, 8, 60, mem);
// 	fprintf(fp, "Window Num,SCM,SCU,Ada CM,IACM\n");

// 	for (int k = 3; k <= 8; ++k) {
// 		int win = 1 << k;
// 		cfg.ds_win_num = win;
// 		cfg.win_num = win;
// 		cfg.win_time = 480.0 / cfg.win_num;
// 		initialize_win_num_test();
// 		fprintf(fp, "%d,%.8f,%.8f,%.8f,%.8f\n", win, cnt_test_win_num_ware(build_scm(mem, k)), 
// 			cnt_test_win_num_ware(build_scu(mem, k)), 
// 			cnt_test_win_num_ware(build_adacm(mem)),
// 			cnt_test_win_num_ware(build_iacm(mem)));
// 		fflush(fp);
// 	}
// 	fclose(fp);
// }

// void figure8_3() {
// 	FILE *fp = fopen("figure8_3.csv", "w");
// 	const int mem = MB(300);
// 	cfg = config("CAIDA", 8, 8, 60, mem);
// 	fprintf(fp, "Window Num,SCM,SCU,Ada CM,IACM\n");

// 	for (int k = 3; k <= 8; ++k) {
// 		int win = 1 << k;
// 		cfg.ds_win_num = win;
// 		cfg.win_num = win;
// 		cfg.win_time = 480.0 / cfg.win_num;
// 		initialize_win_num_test();
// 		fprintf(fp, "%d,%.8f,%.8f,%.8f,%.8f\n", win, cnt_test_win_num_waae(build_scm(mem, k)), 
// 			cnt_test_win_num_waae(build_scu(mem, k)), 
// 			cnt_test_win_num_waae(build_adacm(mem)),
// 			cnt_test_win_num_waae(build_iacm(mem)));
// 		fflush(fp);
// 	}
// 	fclose(fp);
// }

void figure9_1(const char* file_name) {
	FILE *fp = fopen(file_name, "w");
	int mem = 10*1024*1024;
	cfg = config("CAIDA", 8, 8, 60, mem);
	fprintf(fp, "Window Time(s),SCM,SBF,PBF,IABF\n");
	for (int tim = 10; tim <= 60; tim += 10) {
		initialize(true, tim);
		fprintf(fp, "%d,%.8f,%.8f,%.8f\n", tim, bf_test_wfpr(build_sbf(mem)),
			bf_test_wfpr(build_pbf(mem)), bf_test_wfpr(build_iabf(mem)));
		fflush(fp);
	}
	fclose(fp);
}

// void figure9_2() {
// 	FILE *fp = fopen("figure9_2.csv", "w");
// 	cfg = config("CAIDA", 8, 8, 60, MB(300));
// 	fprintf(fp, "Window Time(s),SCM,SCU,Ada CM,IACM\n");
// 	for (int tim = 10; tim <= 60; tim += 10) {
// 		initialize(true, tim);

// 		fprintf(fp, "%d,%.8f,%.8f,%.8f,%.8f\n", tim, 
// 			cnt_test_ware(build_scm(cfg.memory)), cnt_test_ware(build_scu(cfg.memory)),
// 			cnt_test_ware(build_adacm(cfg.memory)), cnt_test_ware(build_iacm(cfg.memory)));
// 		fflush(fp);
// 	}
// 	fclose(fp);
// }

// void figure9_3() {
// 	FILE *fp = fopen("figure9_3.csv", "w");
// 	cfg = config("CAIDA", 8, 8, 60, MB(300));
// 	fprintf(fp, "Window Time(s),SCM,SCU,Ada CM,IACM\n");
// 	for (int tim = 10; tim <= 60; tim += 10) {
// 		initialize(true, tim);

// 		fprintf(fp, "%d,%.8f,%.8f,%.8f,%.8f\n", tim, 
// 			cnt_test_waae(build_scm(cfg.memory)), cnt_test_waae(build_scu(cfg.memory)),
// 			cnt_test_waae(build_adacm(cfg.memory)), cnt_test_waae(build_iacm(cfg.memory)));
// 		fflush(fp);
// 	}
// 	fclose(fp);
// }

// void figure10_1() {
// 	cfg = config("CAIDA", 8, 8, 60, MB(30));
	
// 	initialize();
// 	FILE *fp = fopen("figure10_1.csv", "w");
// 	double *fpr = new double[cfg.ds_win_num + 1];
// 	fprintf(stderr, "read complete\n");

// 	fprintf(fp, "query length,");
// 	for (int i = 1; i <= cfg.ds_win_num; ++i)
// 		fprintf(fp, "%d,", i);
// 	fprintf(fp, "\n");

// 	bf_test_multi_fpr(build_sbf(cfg.memory), fpr);
// 	output_line("sbf", fpr, fp);

// 	bf_test_multi_fpr(build_pbf(cfg.memory), fpr);
// 	output_line("pbf", fpr, fp);
	
// 	bf_test_multi_fpr(build_iabf(cfg.memory), fpr);
// 	output_line("iabf", fpr, fp);

// 	fclose(fp);
// }

// void figure10_2() {
// 	cfg = config("CAIDA", 8, 8, 60, MB(300));

// 	initialize(true);
// 	FILE *fp = fopen("figure10_2.csv", "w");
// 	double *are = new double[cfg.win_num + 1];

// 	fprintf(fp, "window id,");
// 	for (int i = 1; i <= cfg.ds_win_num; ++i)
// 		fprintf(fp, "%d,", i);
// 	fprintf(fp, "\n");

// 	fprintf(stderr, "Testing Stair CM\n");
// 	cnt_test_multi_are(build_scm(cfg.memory), are);
// 	output_line("scm", are, fp);

// 	fprintf(stderr, "Testing Stair CU\n");
// 	cnt_test_multi_are(build_scu(cfg.memory), are);
// 	output_line("scu", are, fp);

// 	fprintf(stderr, "Testing Ada CM\n");
// 	cnt_test_multi_are(build_adacm(cfg.memory), are);
// 	output_line("adacm", are, fp);

// 	fprintf(stderr, "Testing IACM\n");
// 	cnt_test_multi_are(build_iacm(cfg.memory), are);
// 	output_line("iacm", are, fp);
// 	fclose(fp);
// }

// void figure10_3() {
// 	cfg = config("CAIDA", 8, 8, 60, MB(300));

// 	initialize(true);
// 	FILE *fp = fopen("figure10_3.csv", "w");
// 	double *are = new double[cfg.win_num + 1];

// 	fprintf(fp, "window id,");
// 	for (int i = 1; i <= cfg.ds_win_num; ++i)
// 		fprintf(fp, "%d,", i);
// 	fprintf(fp, "\n");

// 	fprintf(stderr, "Testing Stair CM\n");
// 	cnt_test_multi_aae(build_scm(cfg.memory), are);
// 	output_line("scm", are, fp);

// 	fprintf(stderr, "Testing Stair CU\n");
// 	cnt_test_multi_aae(build_scu(cfg.memory), are);
// 	output_line("scu", are, fp);

// 	fprintf(stderr, "Testing Ada CM\n");
// 	cnt_test_multi_aae(build_adacm(cfg.memory), are);
// 	output_line("adacm", are, fp);

// 	fprintf(stderr, "Testing IACM\n");
// 	cnt_test_multi_aae(build_iacm(cfg.memory), are);
// 	output_line("iacm", are, fp);
// 	fclose(fp);
// }

void figure11_1(const char* file_name) {
	// cfg = config("CAIDA", 32, 32, 30, 15*1024*1024);
	
	initialize(true);
	// FILE *fp = fopen("figure11_1.csv", "w");
	FILE *fp = fopen(file_name, "w");
	double *qcnt = new double[cfg.win_num + 1];
	fprintf(stderr, "read complete\n");

	fprintf(fp, "window id,");
	for (int i = 1; i <= cfg.win_num; ++i)
		fprintf(fp, "%d,", i);
	fprintf(fp, "\n");

	bf_test_qcnt(build_sbf(cfg.memory,5), qcnt);
	output_line("sbf", qcnt, fp);

	bf_test_qcnt(build_pbf(cfg.memory), qcnt);
	output_line("pbf", qcnt, fp);
	
	bf_test_qcnt(build_iabf(cfg.memory), qcnt);
	output_line("iabf", qcnt, fp);

	fclose(fp);
}

// void figure11_2() {
// 	cfg = config("webpage", 16, 16, 1, 120*1024*1024);
// 	initialize(true);
// 	FILE *fp = fopen("figure11_2.csv", "w");
	
// 	double *qcnt = new double[cfg.win_num + 1];

// 	fprintf(fp, "window id,");
// 	for (int i = 1; i <= cfg.ds_win_num; ++i)
// 		fprintf(fp, "%d,", i);
// 	fprintf(fp, "\n");

// 	cnt_test_qcnt(build_scm(cfg.memory,5), qcnt);
// 	output_line("scm", qcnt, fp);

// 	cnt_test_qcnt(build_scu(cfg.memory,5), qcnt);
// 	output_line("scu", qcnt, fp);

// 	cnt_test_qcnt(build_adacm(cfg.memory), qcnt);
// 	output_line("adacm", qcnt, fp);

// 	cnt_test_qcnt(build_iacm(cfg.memory), qcnt);
// 	output_line("iacm", qcnt, fp);
// 	fclose(fp);
// }

vector<function<framework*(int)> > all_dasketch_framwork, dasketch_framework_list[10];
vector<function<framework*(int)> > all_hll_framwork, hll_framework_list[10];
vector<function<framework*(int)> > all_elastic_framwork, elastic_framework_list[10];
vector<function<framework*(int)> > all_tower_framwork, tower_framework_list[10];
vector<function<framework*(int)> > all_cmcu_framwork, cmcu_framework_list[10];
void prepare_all_framework(){
	// DA
	all_dasketch_framwork.push_back([](int memory){ return build_sda(memory); });
	all_dasketch_framwork.push_back(build_hda);
	all_dasketch_framwork.push_back(build_adada);
	for (int i = 0; i < 10; i++){
		dasketch_framework_list[i].push_back([i](int memory){ return build_sda(memory, i); });
		dasketch_framework_list[i].push_back(build_hda);
		dasketch_framework_list[i].push_back(build_adada);
	}
	// HLL
	all_hll_framwork.push_back([](int memory){ return build_shll(memory); });
	all_hll_framwork.push_back(build_hhll);
	for (int i = 0; i < 10; i++){
		hll_framework_list[i].push_back([i](int memory){ return build_shll(memory, i); });
		hll_framework_list[i].push_back(build_hhll);
	}
	// Elastic
	all_elastic_framwork.push_back([](int memory){ return build_selastic(memory); });
	all_elastic_framwork.push_back(build_helastic);
	all_elastic_framwork.push_back(build_adaelastic);
	for (int i = 0; i < 10; i++){
		elastic_framework_list[i].push_back([i](int memory){ return build_selastic(memory, i); });
		elastic_framework_list[i].push_back(build_helastic);
		elastic_framework_list[i].push_back(build_adaelastic);
	}
	// Tower
	all_tower_framwork.push_back([](int memory){ return build_stower(memory); });
	all_tower_framwork.push_back(build_htower);
	all_tower_framwork.push_back(build_adatower);
	for (int i = 0; i < 10; i++){
		tower_framework_list[i].push_back([i](int memory){ return build_stower(memory, i); });
		tower_framework_list[i].push_back(build_htower);
		tower_framework_list[i].push_back(build_adatower);
	}
	// CMCU
	all_cmcu_framwork.push_back([](int memory){ return build_scm(memory); });
	all_cmcu_framwork.push_back([](int memory){ return build_scu(memory); });
	all_cmcu_framwork.push_back(build_adacm);
	all_cmcu_framwork.push_back(build_iacm);
	for (int i = 0; i < 10; i++){
		cmcu_framework_list[i].push_back([i](int memory){ return build_scm(memory, i); });
		cmcu_framework_list[i].push_back([i](int memory){ return build_scu(memory, i); });
		cmcu_framework_list[i].push_back(build_adacm);
		cmcu_framework_list[i].push_back(build_iacm);
	}
}

void TestDifferentMemory();
void TestErrorGradualness();
void TestTimeStability();
void TestAMA();
void TestDifferentWindowTime();
void TestDifferentWindowNumber();
void TestQueryLength();

void figure12_1();
void figure12_2();

int main() {
	prepare_all_framework();
	srand(1214);

	figure12_2();
	return 0;

	// TestDifferentWindowTime();
	// TestDifferentWindowNumber();
	TestErrorGradualness();
	// TestTimeStability();

	// TestDifferentMemory();

	// TestAMA();

	// TestQueryLength();
	
	return 0;
}

void Newfigure_fixed_config_result(const char* file_name, int current_config, 
	function<double(framework*)> test_function, 
	const vector<std::function<framework*(int)>>& build_function){
	
	FILE *fp = fopen(file_name, "a");
	fprintf(fp, "%d", current_config);

	for (int i = 0; i < build_function.size(); i++){
		double result = 0;
		for (int t = 0; t < TEST_REPEAT_TIME; t++){
			framework *sketch = build_function[i](cfg.memory);
			result += test_function(sketch);
			delete sketch;
		}
		result /= TEST_REPEAT_TIME;
		fprintf(fp, ",%.8f", result);
	}
	fprintf(fp, "\n");
	fclose(fp);
}

void TestDifferentWindowTime(){
	cfg = config(SELECTED_DATASET, 32, 32, ((double)CAIDA_DATA_RANGE) / 32, MB(4));
	int start_tim = 10, end_tim = 60, gap_tim = 10;
	for (int tim = start_tim; tim <= end_tim; tim += gap_tim) {
		initialize(true, tim);
	#ifdef TEST_DA
		cfg.memory = DA_DEFAULT_MEMORY;
		if (tim == start_tim){
			FILE *fp = fopen("2_2_DA_WARE_wintime.csv", "w");
			fprintf(fp, "Window Time(s),SDA,HDA,AdaDA\n");
			fclose(fp);
			fp = fopen("2_2_DA_WAAE_wintime.csv", "w");
			fprintf(fp, "Window Time(s),SDA,HDA,AdaDA\n");
			fclose(fp);
			fp = fopen("2_2_DA_WF1_wintime.csv", "w");
			fprintf(fp, "Window Time(s),SDA,HDA,AdaDA\n");
			fclose(fp);
		}
		Newfigure_fixed_config_result("2_2_DA_WARE_wintime.csv", tim, topk_test_ware, all_dasketch_framwork);
		Newfigure_fixed_config_result("2_2_DA_WAAE_wintime.csv", tim, topk_test_waae, all_dasketch_framwork);
		Newfigure_fixed_config_result("2_2_DA_WF1_wintime.csv", tim, topk_test_wf1, all_dasketch_framwork);
	#endif
	#ifdef TEST_HLL
		// HLLFigure();
	#endif
	#ifdef TEST_ELASTIC
		cfg.memory = ELASTIC_DEFAULT_MEMORY;
		if (tim == start_tim){
			FILE *fp = fopen("2_2_Elastic_WARE_wintime.csv", "w");
			fprintf(fp, "Window Time(s),SElastic,HElastic,AdaElastic\n");
			fclose(fp);
			fp = fopen("2_2_Elastic_WAAE_wintime.csv", "w");
			fprintf(fp, "Window Time(s),SElastic,HElastic,AdaElastic\n");
			fclose(fp);
			fp = fopen("2_2_Elastic_WF1_wintime.csv", "w");
			fprintf(fp, "Window Time(s),SElastic,HElastic,AdaElastic\n");
			fclose(fp);
		}
		Newfigure_fixed_config_result("2_2_Elastic_WARE_wintime.csv", tim, topk_test_ware, all_elastic_framwork);
		Newfigure_fixed_config_result("2_2_Elastic_WAAE_wintime.csv", tim, topk_test_waae, all_elastic_framwork);
		Newfigure_fixed_config_result("2_2_Elastic_WF1_wintime.csv", tim, topk_test_wf1, all_elastic_framwork);
	#endif
	#ifdef TEST_TOWER
		cfg.memory = TOWER_DEFAULT_MEMORY;
		if (tim == start_tim){
			FILE *fp = fopen("1_2_Tower_WARE_wintime.csv", "w");
			fprintf(fp, "Window Time(s),STower,HTower,AdaTower\n");
			fclose(fp);
			fp = fopen("1_2_Tower_WAAE_wintime.csv", "w");
			fprintf(fp, "Window Time(s),STower,HTower,AdaTower\n");
			fclose(fp);
		}
		Newfigure_fixed_config_result("1_2_Tower_WARE_wintime.csv", tim, cnt_test_ware, all_tower_framwork);
		Newfigure_fixed_config_result("1_2_Tower_WAAE_wintime.csv", tim, cnt_test_waae, all_tower_framwork);
	#endif
	#ifdef TEST_BF
		// BloomFilter();
	#endif
	}
}

void TestDifferentWindowNumber(){
	cfg = config(SELECTED_DATASET, 32, 32, ((double)CAIDA_DATA_RANGE) / 32, MB(4));
	int start_num = 8, end_num = 256;
	for (int num = start_num, stair_level_number = 3; num <= end_num; num <<= 1, stair_level_number++) {
		cfg.ds_win_num = num;
		cfg.win_num = num;
		cfg.win_time = ((double)CAIDA_DATA_RANGE) / cfg.win_num;
		initialize_win_num_test();
	#ifdef TEST_DA
		cfg.memory = DA_DEFAULT_MEMORY;
		if (num == start_num){
			FILE *fp = fopen("2_2_DA_WARE_winnum.csv", "w");
			fprintf(fp, "Window Number,SDA,HDA,AdaDA\n");
			fclose(fp);
			fp = fopen("2_2_DA_WAAE_winnum.csv", "w");
			fprintf(fp, "Window Number,SDA,HDA,AdaDA\n");
			fclose(fp);
			fp = fopen("2_2_DA_WF1_winnum.csv", "w");
			fprintf(fp, "Window Number,SDA,HDA,AdaDA\n");
			fclose(fp);
		}
		Newfigure_fixed_config_result("2_2_DA_WARE_winnum.csv", num, topk_test_ware, dasketch_framework_list[stair_level_number]);
		Newfigure_fixed_config_result("2_2_DA_WAAE_winnum.csv", num, topk_test_waae, dasketch_framework_list[stair_level_number]);
		Newfigure_fixed_config_result("2_2_DA_WF1_winnum.csv", num, topk_test_wf1, dasketch_framework_list[stair_level_number]);
	#endif
	#ifdef TEST_HLL
		// HLLFigure();
	#endif
	#ifdef TEST_ELASTIC
		cfg.memory = ELASTIC_DEFAULT_MEMORY;
		if (num == start_num){
			FILE *fp = fopen("2_2_Elastic_WARE_winnum.csv", "w");
			fprintf(fp, "Window Number,SElastic,HElastic,AdaElastic\n");
			fclose(fp);
			fp = fopen("2_2_Elastic_WAAE_winnum.csv", "w");
			fprintf(fp, "Window Number,SElastic,HElastic,AdaElastic\n");
			fclose(fp);
			fp = fopen("2_2_Elastic_WF1_winnum.csv", "w");
			fprintf(fp, "Window Number,SElastic,HElastic,AdaElastic\n");
			fclose(fp);
		}
		Newfigure_fixed_config_result("2_2_Elastic_WARE_winnum.csv", num, topk_test_ware, elastic_framework_list[stair_level_number]);
		Newfigure_fixed_config_result("2_2_Elastic_WAAE_winnum.csv", num, topk_test_waae, elastic_framework_list[stair_level_number]);
		Newfigure_fixed_config_result("2_2_Elastic_WF1_winnum.csv", num, topk_test_wf1, elastic_framework_list[stair_level_number]);
	#endif
	#ifdef TEST_TOWER
		cfg.memory = TOWER_DEFAULT_MEMORY;
		if (num == start_num){
			FILE *fp = fopen("1_2_Tower_WARE_winnum.csv", "w");
			fprintf(fp, "Window Number,STower,HTower,AdaTower\n");
			fclose(fp);
			fp = fopen("1_2_Tower_WAAE_winnum.csv", "w");
			fprintf(fp, "Window Number,STower,HTower,AdaTower\n");
			fclose(fp);
		}
		Newfigure_fixed_config_result("1_2_Tower_WARE_winnum.csv", num, cnt_test_ware, tower_framework_list[stair_level_number]);
		Newfigure_fixed_config_result("1_2_Tower_WAAE_winnum.csv", num, cnt_test_waae, tower_framework_list[stair_level_number]);
	#endif
	#ifdef TEST_BF
		// BloomFilter();
	#endif
	}
}

void Newfigure_memory_fixed(const char* file_name, 
	function<void(framework*, double*)> test_function, 
	const vector<std::function<framework*(int)>>& build_function){
	
	FILE *fp = fopen(file_name, "w");
	
	double *qcnt = new double[cfg.win_num + 1];

	fprintf(fp, "window id,");
	for (int i = 1; i <= cfg.ds_win_num; ++i)
		fprintf(fp, "%d,", i);
	fprintf(fp, "\n");

	for (int i = 0; i < build_function.size(); i++){
		double *result = new double[cfg.ds_win_num + 1];
		memset(result, 0, sizeof(double) * (cfg.ds_win_num + 1));
		for (int t = 0; t < TEST_REPEAT_TIME; t++){
			framework *sketch = build_function[i](cfg.memory);
			test_function(sketch, qcnt);
			for (int j = 1; j <= cfg.ds_win_num; j++)
				result[j] += qcnt[j];
			if (t == TEST_REPEAT_TIME - 1){
				for (int j = 1; j <= cfg.ds_win_num; j++)
					result[j] /= TEST_REPEAT_TIME;
				output_line(sketch->name(), result, fp);
			}
			delete sketch;
		}
		delete[] result;
	}
	fclose(fp);
	delete[] qcnt;
}

void TestErrorGradualness(){
	cfg = config(SELECTED_DATASET, 32, 32, ((double)CAIDA_DATA_RANGE) / 32, MB(120));
	initialize(true);
#ifdef TEST_DA
	// DA
	cfg.memory = DA_DEFAULT_MEMORY;
	Newfigure_memory_fixed("2_1_1_DA_f1.csv",topk_test_f1,all_dasketch_framwork);
	Newfigure_memory_fixed("2_1_1_DA_ARE.csv",topk_test_are,all_dasketch_framwork);
#endif

#ifdef TEST_HLL
	// HLL
	cfg.memory = KB(64);
	Newfigure_memory_fixed("3_1_1_Hyperloglog_ARE.csv",cardinal_test_are,all_hll_framwork);
#endif

#ifdef TEST_ELASTIC
	// Elastic
	cfg.memory = ELASTIC_DEFAULT_MEMORY;
	Newfigure_memory_fixed("2_1_1_Elastic_f1.csv",topk_test_f1,all_elastic_framwork);
	Newfigure_memory_fixed("2_1_1_Elastic_ARE.csv",topk_test_are,all_elastic_framwork);
#endif

#ifdef TEST_TOWER
	// Tower
	cfg.memory = TOWER_DEFAULT_MEMORY;
	Newfigure_memory_fixed("1_1_1_Tower_ARE.csv",cnt_test_are,all_tower_framwork);
	Newfigure_memory_fixed("1_1_1_Tower_AAE.csv",cnt_test_aae,all_tower_framwork);
#endif

#ifdef TEST_CMCU
	// CMCU
	cfg.memory = CMCU_DEFAULT_MEMORY;
	Newfigure_memory_fixed("1_1_1_CMCU_ARE.csv",cnt_test_are,all_cmcu_framwork);
	Newfigure_memory_fixed("1_1_1_CMCU_AAE.csv",cnt_test_aae,all_cmcu_framwork);
#endif

#ifdef TEST_BF
	// BF
	cfg.memory = MB(4);
	figure1("4_1_1_BF_fpr.csv");
#endif
}

void TestTimeStability(){
	cfg = config(SELECTED_DATASET, 64, 64, ((double)CAIDA_DATA_RANGE) / 64, MB(120));
	initialize(true);
#ifdef TEST_DA
	// DA
	cfg.memory = DA_DEFAULT_MEMORY;
	Newfigure_memory_fixed("2_1_2_DA_f1.csv",topk_test_stability_f1,all_dasketch_framwork);
	Newfigure_memory_fixed("2_1_2_DA_ARE.csv",topk_test_stability_are,all_dasketch_framwork);
#endif

#ifdef TEST_HLL
	// HLL
	cfg.memory = KB(64);
	Newfigure_memory_fixed("3_1_2_Hyperloglog_ARE.csv",cardinal_test_stability_are,all_hll_framwork);
#endif

#ifdef TEST_ELASTIC
	// Elastic
	cfg.memory = ELASTIC_DEFAULT_MEMORY;
	Newfigure_memory_fixed("2_1_2_Elastic_f1.csv",topk_test_stability_f1,all_elastic_framwork);
	Newfigure_memory_fixed("2_1_2_Elastic_ARE.csv",topk_test_stability_are,all_elastic_framwork);
#endif

#ifdef TEST_TOWER
	// Tower
	cfg.memory = TOWER_DEFAULT_MEMORY;
	Newfigure_memory_fixed("1_1_2_Tower_ARE.csv",cnt_test_stability_are,all_tower_framwork);
	Newfigure_memory_fixed("1_1_2_Tower_AAE.csv",cnt_test_stability_aae,all_tower_framwork);
#endif

#ifdef TEST_CMCU
	// CMCU
	cfg.memory = CMCU_DEFAULT_MEMORY;
	Newfigure_memory_fixed("1_1_2_CMCU_ARE.csv",cnt_test_stability_are,all_cmcu_framwork);
	Newfigure_memory_fixed("1_1_2_CMCU_AAE.csv",cnt_test_stability_aae,all_cmcu_framwork);
#endif

#ifdef TEST_BF
	// BF
	// // cfg = config("webpage", 16, 8, 60, MB(2));
	cfg.memory = MB(2);
	figure2("4_1_2_BF_fpr.csv");
#endif
}

void TestAMA(){
	cfg = config(SELECTED_DATASET, 32, 32, ((double)CAIDA_DATA_RANGE) / 32, MB(120));
	initialize(true);
#ifdef TEST_DA
	// DA
	cfg.memory = DA_DEFAULT_MEMORY;
	Newfigure_memory_fixed("2_3_1_DA_AMA.csv",topk_test_qcnt,all_dasketch_framwork);
#endif

#ifdef TEST_HLL
	// HLL
	cfg.memory = MB(15);
	Newfigure_memory_fixed("3_3_1_Hyperloglog_AMA.csv",cnt_test_qcnt_se,all_hll_framwork);
#endif

#ifdef TEST_ELASTIC
	// Elastic
	cfg.memory = ELASTIC_DEFAULT_MEMORY;
	Newfigure_memory_fixed("2_3_1_Elastic_AMA.csv",topk_test_qcnt,all_elastic_framwork);
#endif

#ifdef TEST_TOWER
	// Tower
	cfg.memory = TOWER_DEFAULT_MEMORY;
	Newfigure_memory_fixed("1_3_1_Tower_AMA.csv",cnt_test_qcnt_se,all_tower_framwork);
#endif

#ifdef TEST_CMCU
	// CMCU
	cfg.memory = CMCU_DEFAULT_MEMORY;
	Newfigure_memory_fixed("1_3_1_CMCU_AMA.csv",cnt_test_qcnt_se,all_cmcu_framwork);
#endif
	
#ifdef TEST_BF
	// BF
	cfg.memory = MB(15);
	figure11_1("4_3_1_BF_AMA.csv");
#endif
}

void TestQueryLength(){
	cfg = config(SELECTED_DATASET, 32, 32, ((double)CAIDA_DATA_RANGE) / 32, MB(120));
	initialize(true);
#ifdef TEST_TOWER
	// Tower
	cfg.memory = TOWER_DEFAULT_MEMORY;
	Newfigure_memory_fixed("1_2_Tower_QueryLength_ARE.csv",cnt_test_multi_are,all_tower_framwork);
	Newfigure_memory_fixed("1_2_Tower_QueryLength_AAE.csv",cnt_test_multi_aae,all_tower_framwork);
#endif
}

void Newfigure(const char* file_name, const char* csv_first_line, int m_begin, int m_end, int m_step, 
	function<double(framework*)> test_function, 
	const vector<std::function<framework*(int)>>& build_function){

	FILE *fp = fopen(file_name, "w");
	// initialize(true);
	fprintf(fp, csv_first_line);
	for (int mem = m_begin; mem <= m_end; mem += m_step) {
		cfg.memory = mem;
		fprintf(fp, "%d", mem/1024/1024);
		for (int i = 0; i < build_function.size(); i++){
			double result = 0;
			for (int t = 0; t < TEST_REPEAT_TIME; t++){
				framework *sketch = build_function[i](mem);
				result += test_function(sketch);
				delete sketch;
			}
			result /= TEST_REPEAT_TIME;
			fprintf(fp, ",%.8f", result);
		}
		fprintf(fp, "\n");
		fflush(fp);
	}
	fclose(fp);
}


void DASketchFigure(){
	// Newfigure("rb.csv","Memory(MB),SDA,HDA,AdaDA\n",
	// 	MB(50), MB(100), MB(10), ada_test_diff_window_f1, all_dasketch_framwork);
	Newfigure("2_2_DA_WF1.csv","Memory(MB),SDA,HDA,AdaDA\n",
		MB(5), MB(25), MB(5), topk_test_wf1, all_dasketch_framwork);
	Newfigure("2_2_DA_WARE.csv","Memory(MB),SDA,HDA,AdaDA\n",
		MB(5), MB(25), MB(5), topk_test_ware, all_dasketch_framwork);
	// Newfigure("2_3_2_DA_throughput.csv","Memory(MB),SDA,HDA,AdaDA\n",
	// 	MB(120), MB(120), MB(5), test_speed, all_dasketch_framwork);
}


void HLLFigure(){
	Newfigure("3_2_Hyperloglog_WARE.csv","Memory(MB),SHLL,HHLL\n",
		KB(64), KB(512), KB(64), cardinal_test_ware, all_hll_framwork);
	Newfigure("3_3_2_Hyperloglog_throughput.csv","Memory(MB),SHLL,HHLL\n",
		MB(15), MB(15), MB(5), test_speed, all_hll_framwork);
}


void ElasticFigure(){
	Newfigure("2_2_Elastic_WF1.csv","Memory(MB),SElastic,HElastic,AdaElastic\n",
		MB(15), MB(35), MB(5), topk_test_wf1, all_elastic_framwork);
	// Newfigure("2_2_Elastic_WARE.csv","Memory(MB),SElastic,HElastic,AdaElastic\n",
	// 	MB(15), MB(35), MB(5), topk_test_ware, all_elastic_framwork);
	// Newfigure("2_3_2_Elastic_throughput.csv","Memory(MB),SElastic,HElastic,AdaElastic\n",
	// 	MB(120), MB(120), MB(5), test_speed, all_elastic_framwork);
}


void TowerFigure(){
	Newfigure("1_2_Tower_WARE.csv","Memory(MB),STower,HTower,AdaTower,SCM,SCU,AdaCM,HCM\n",
		MB(5), MB(30), MB(5), cnt_test_ware, all_tower_framwork);
	Newfigure("1_3_2_Tower_throughput.csv","Memory(MB),STower,HTower,AdaTower,SCM,SCU,AdaCM,HCM\n",
		MB(30), MB(30), MB(5), test_speed, all_tower_framwork);
}

void BloomFilter(){
	figure5("4_2_BF_wfpr_memory.csv", MB(2), MB(16), MB(2));
	cfg.memory = MB(20);
	figure8_1("4_2_BF_wfpr_time_period.csv");
	cfg.memory = MB(10);
	figure9_1("4_2_BF_wfpr_query_length.csv");
	// bf_test_wfpr
	// bf_test_multi_fpr
	// bf_test_win_num_wfpr
	// test_speed_old
	FILE *fp = fopen("4_3_2_BF_throughput.csv", "w");
	// initialize(true);
	fprintf(fp, "Memory(MB),STower,HTower,AdaTower\n");

	cfg.memory = MB(15);
	fprintf(fp, "%d", cfg.memory/1024/1024);
	fprintf(fp, ",%.8f", test_speed_old(build_sbf(cfg.memory)));
	fprintf(fp, ",%.8f", test_speed_old(build_pbf(cfg.memory)));
	fprintf(fp, ",%.8f", test_speed_old(build_iabf(cfg.memory)));
	fprintf(fp, "\n");
	
	fflush(fp);
	fclose(fp);
}

void TestDifferentMemory(){
	cfg = config(SELECTED_DATASET, 32, 32, ((double)CAIDA_DATA_RANGE) / 32, MB(4));
	initialize(true);
#ifdef TEST_DA
	DASketchFigure();
#endif
#ifdef TEST_HLL
	HLLFigure();
#endif
#ifdef TEST_ELASTIC
	ElasticFigure();
#endif
#ifdef TEST_TOWER
	TowerFigure();
#endif
#ifdef TEST_CMCU
	// CMCUFigure();
#endif
#ifdef TEST_BF
	BloomFilter();
#endif
}










void figure12_1() {
	cfg = config("CAIDA", 32, 32, 1, 15*1024*1024);
	
	initialize(true);
	FILE *fp = fopen("figure12_1.txt", "w");
	double *qcnt = new double[cfg.win_num + 1];
	fprintf(stderr, "read complete\n");

	fprintf(fp, "window id,");
	for (int i = 1; i <= cfg.win_num; ++i)
		fprintf(fp, "%d,", i);
	fprintf(fp, "\n");

	fprintf(fp, "SBF: %.6f\n", test_speed_old(build_sbf(cfg.memory,5)));
	fprintf(fp, "PBF: %.6f\n", test_speed_old(build_pbf(cfg.memory)));
	fprintf(fp, "IABF: %.6f\n", test_speed_old(build_iabf(cfg.memory)));

	fclose(fp);
}

void figure12_2() {
	cfg = config("CAIDA", 32, 32, 1, 120*1024*1024);
	initialize(true);
	
	FILE *fp = fopen("figure12_2.txt", "w");
	double *qcnt = new double[cfg.win_num + 1];
	fprintf(stderr, "read complete\n");

	fprintf(fp, "window id,");
	for (int i = 1; i <= cfg.win_num; ++i)
		fprintf(fp, "%d,", i);
	fprintf(fp, "\n");

	fprintf(fp, "SCM: %.6f\n", test_speed(build_scm(cfg.memory,5)));
	fprintf(fp, "SCU: %.6f\n", test_speed(build_scu(cfg.memory,5)));
	fprintf(fp, "ADACM: %.6f\n", test_speed(build_adacm(cfg.memory)));
	fprintf(fp, "IACM: %.6f\n", test_speed(build_iacm(cfg.memory)));

	fclose(fp);

}