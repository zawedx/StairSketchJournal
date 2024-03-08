#include "config.hpp"
#include "utils.hpp"
#include "file_reader.hpp"
#include "stair_bf.hpp"
#include "stair_cm.hpp"
#include "stair_cu.hpp"
#include "pbf.hpp"
#include "hokusai.hpp"
#include "adacm.hpp"
#include "test.cpp"
#include <algorithm>


void initialize(bool save = false, double win_time = cfg.win_time) {
	for (int i = 1; i <= cfg.win_num; ++i)
		win_data[i].clear(), win_set[i].clear();

	file_reader* fr = new file_reader(cfg);
	HashMap* hmap = new HashMap;
	elem_t e; double ts, ts_begin;
	fr->read(e, ts); ts_begin = ts;
	int count_read = 1;
	unordered_map<elem_t, int> count_win32;
	int count_win32_total = 0;
	for (int i = 1; i <= cfg.win_num; ) {
		hmap->add(i, e);
		if (save) {
			win_data[i].push_back(e);
			win_set[i][e]++;
			elem_set.insert(e);
		}
		if (!fr->read(e, ts)) break;
		if (i == 32) count_win32[e] = 1;
		if (i == 32) count_win32_total ++;
		count_read++;
		if (count_read % 4000000 == 0) 
			fprintf(stderr, "\"i=%d, ts-tsbegin=%.4lf\"\n", i, ts - ts_begin);
		if (ts - ts_begin > i * win_time)
			i++;
	}
	fprintf(stderr, "\"win32=%d\" ", (int)count_win32.size());
	fprintf(stderr, "\"win32total=%d\" ", count_win32_total);
	fprintf(stderr, "\"total read=%d\" ", count_read);
	if (elems != nullptr) delete elems;
	elem_cnt = hmap->all_elements(elems);
	delete fr;
	delete hmap;

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
		fprintf(fp, "%.6f%c", arr[i], i == n ? '\n' : ',');
	fflush(fp);
}

int MB(int n) { return n * 1024 * 1024; }

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

void figure1_1() {
	cfg = config("CAIDA", 8, 8, 60, MB(5));
	figure1("figure1_1.csv");
}

void figure1_2() {
	cfg = config("zipf", 8, 8, 60, MB(10));
	figure1("figure1_2.csv");
}

void figure1_3() {
	cfg = config("webpage", 32, 32, 60, MB(4));
	figure1("figure1_3.csv");
}

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

void figure2_1() {
	cfg = config("CAIDA", 16, 8, 60, MB(3));
	figure2("figure2_1.csv");
}

void figure2_2() {
	cfg = config("zipf", 16, 8, 60, MB(5));
	figure2("figure2_2.csv");
}

void figure2_3() {
	cfg = config("webpage", 16, 8, 60, MB(1));
	figure2("figure2_3.csv");
}

template<class sketch> void test3(const char* name, sketch *sk, double *are, FILE *fp) {
	cnt_test_are(sk, are);
	output_line(name, are, fp);
}

void figure3(const char* file_name) {
	initialize(true);
	FILE *fp = fopen(file_name, "w");
	double *are = new double[cfg.win_num + 1];

	fprintf(fp, "window id,");
	for (int i = 1; i <= cfg.ds_win_num; ++i)
		fprintf(fp, "%d,", i);
	fprintf(fp, "\n");

	test3("scm",   build_scm(cfg.memory),   are, fp);
	test3("scu",   build_scu(cfg.memory),   are, fp);
	test3("adacm", build_adacm(cfg.memory), are, fp);
	test3("iacm",  build_iacm(cfg.memory),  are, fp);

	fclose(fp);
}

void figure3_1() {
	cfg = config("CAIDA", 8, 8, 60, 80*1024*1024);
	figure3("figure3_1_.csv");
}

void figure3_2() {
	cfg = config("zipf", 8, 8, 60, 200*1024*1024);
	figure3("figure3_2.csv");
}

void figure3_3() {
	cfg = config("webpage", 8, 8, 60, 50*1024*1024);
	figure3("figure3_3.csv");
}

template<class sketch> void test4(const char* name, sketch *sk, double *are, FILE *fp) {
	cnt_test_stability(sk, are);
	output_line(name, are, fp, cfg.win_num);
}

void figure4(const char* file_name) {
	initialize(true);
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

void figure4_1() {
	cfg = config("CAIDA", 16, 8, 60, MB(60));
	figure4("figure4_1.csv");
}

void figure4_2() {
	cfg = config("zipf", 16, 8, 60, MB(100));
	figure4("figure4_2.csv");
}

void figure4_3() {
	cfg = config("webpage", 16, 8, 60, MB(30));
	figure4("figure4_3.csv");
}

void figure5(const char* file_name, int m_begin, int m_end, int m_step) {
	FILE *fp = fopen(file_name, "w");
	initialize();
	fprintf(fp, "Memory(MB),SBF,PBF,IABF\n");
	for (int mem = m_begin; mem <= m_end; mem += m_step) {
		cfg.memory = mem;
		fprintf(fp, "%d,%.6f,%.6f,%.6f\n", mem/1024/1024, bf_test_wfpr(build_sbf(mem, 5)),
			bf_test_wfpr(build_pbf(mem)), bf_test_wfpr(build_iabf(mem)));
		fflush(fp);
	}
	fclose(fp);
}

void figure5_1() {
	cfg = config("CAIDA", 8, 8, 60, MB(2));
	figure5("figure5_1.csv", MB(2), MB(16), MB(2));
}

void figure5_2() {
	cfg = config("zipf", 8, 8, 60, MB(8));
	figure5("figure5_2.csv", MB(8), MB(32), MB(4));
}

void figure5_3() {
	cfg = config("webpage", 32, 32, 60, MB(2));
	figure5("figure5_3.csv", MB(2), MB(16), MB(2));
}

void figure6(const char* file_name, int m_begin, int m_end, int m_step) {
	FILE *fp = fopen(file_name, "w");
	initialize(true);
	fprintf(fp, "Memory(MB),SCM,SCU,Ada CM,IACM\n");
	for (int mem = m_begin; mem <= m_end; mem += m_step) {
		cfg.memory = mem;
		fprintf(fp, "%d,%.6f,%.6f,%.6f,%.6f\n", mem/1024/1024, 
			cnt_test_ware(build_scm(mem)), cnt_test_ware(build_scu(mem)),
			cnt_test_ware(build_adacm(mem)), cnt_test_ware(build_iacm(mem)));
		fflush(fp);
	}
	fclose(fp);
}

void figure6_1() {
	cfg = config("CAIDA", 8, 8, 60, MB(100));
	figure6("figure6_1.csv", MB(100), MB(180), MB(10));
}

void figure6_2() {
	cfg = config("zipf", 8, 8, 60, MB(160));
	figure6("figure6_2.csv", MB(160), MB(240), MB(10));
}

void figure6_3() {
	cfg = config("webpage", 8, 8, 60, MB(40));
	figure6("figure6_3.csv",MB(40), MB(80), MB(5));
}

void figure7(const char* file_name, int m_begin, int m_end, int m_step) {
	FILE *fp = fopen(file_name, "w");
	initialize(true);
	fprintf(fp, "Memory(MB),SCM,SCU,Ada CM,IACM\n");
	for (int mem = m_begin; mem <= m_end; mem += m_step) {
		cfg.memory = mem;
		fprintf(fp, "%d,%.6f,%.6f,%.6f,%.6f\n", mem/1024/1024, 
			cnt_test_waae(build_scm(mem)), cnt_test_waae(build_scu(mem)),
			cnt_test_waae(build_adacm(mem)), cnt_test_waae(build_iacm(mem)));
		fflush(fp);
	}
	fclose(fp);
}

void figure7_1() {
	cfg = config("CAIDA", 8, 8, 60, MB(100));
	figure7("figure7_1.csv", MB(100), MB(180), MB(10));
}

void figure7_2() {
	cfg = config("zipf", 8, 8, 60, MB(160));
	figure7("figure7_2.csv", MB(160), MB(240), MB(10));
}

void figure7_3() {
	cfg = config("webpage", 8, 8, 60, MB(40));
	figure7("figure7_3.csv", MB(40), MB(80), MB(5));
}

void figure8_1() {
	FILE *fp = fopen("figure8_1.csv", "w");
	const int mem = MB(20);
	cfg = config("CAIDA", 8, 8, 60, mem);
	fprintf(fp, "Window Num,SBF,PBF,IABF\n");
	for (int k = 3; k <= 8; ++k) {
		int win = 1 << k;
		cfg.ds_win_num = win;
		cfg.win_num = win;
		cfg.win_time = 480.0 / cfg.win_num;
		initialize_win_num_test();
		fprintf(fp, "%d,%.6f,%.6f,%.6f\n", win, bf_test_win_num_wfpr(build_sbf(mem, k)), 
			bf_test_win_num_wfpr(build_pbf(mem)), bf_test_win_num_wfpr(build_iabf(mem)));
		fflush(fp);
	}
	fclose(fp);
}

void figure8_2() {
	FILE *fp = fopen("figure8_2.csv", "w");
	const int mem = MB(300);
	cfg = config("CAIDA", 8, 8, 60, mem);
	fprintf(fp, "Window Num,SCM,SCU,Ada CM,IACM\n");

	for (int k = 3; k <= 8; ++k) {
		int win = 1 << k;
		cfg.ds_win_num = win;
		cfg.win_num = win;
		cfg.win_time = 480.0 / cfg.win_num;
		initialize_win_num_test();
		fprintf(fp, "%d,%.6f,%.6f,%.6f,%.6f\n", win, cnt_test_win_num_ware(build_scm(mem, k)), 
			cnt_test_win_num_ware(build_scu(mem, k)), 
			cnt_test_win_num_ware(build_adacm(mem)),
			cnt_test_win_num_ware(build_iacm(mem)));
		fflush(fp);
	}
	fclose(fp);
}

void figure8_3() {
	FILE *fp = fopen("figure8_3.csv", "w");
	const int mem = MB(300);
	cfg = config("CAIDA", 8, 8, 60, mem);
	fprintf(fp, "Window Num,SCM,SCU,Ada CM,IACM\n");

	for (int k = 3; k <= 8; ++k) {
		int win = 1 << k;
		cfg.ds_win_num = win;
		cfg.win_num = win;
		cfg.win_time = 480.0 / cfg.win_num;
		initialize_win_num_test();
		fprintf(fp, "%d,%.6f,%.6f,%.6f,%.6f\n", win, cnt_test_win_num_waae(build_scm(mem, k)), 
			cnt_test_win_num_waae(build_scu(mem, k)), 
			cnt_test_win_num_waae(build_adacm(mem)),
			cnt_test_win_num_waae(build_iacm(mem)));
		fflush(fp);
	}
	fclose(fp);
}

void figure9_1() {
	FILE *fp = fopen("figure9_1.csv", "w");
	int mem = 5*1024*1024;
	cfg = config("CAIDA", 8, 8, 60, mem);
	fprintf(fp, "Window Time(s),SCM,SBF,PBF,IABF\n");
	for (int tim = 10; tim <= 60; tim += 10) {
		initialize(true, tim);
		fprintf(fp, "%d,%.6f,%.6f,%.6f\n", tim, bf_test_wfpr(build_sbf(mem)),
			bf_test_wfpr(build_pbf(mem)), bf_test_wfpr(build_iabf(mem)));
		fflush(fp);
	}
	fclose(fp);
}

void figure9_2() {
	FILE *fp = fopen("figure9_2.csv", "w");
	cfg = config("CAIDA", 8, 8, 60, MB(150));
	fprintf(fp, "Window Time(s),SCM,SCU,Ada CM,IACM\n");
	for (int tim = 10; tim <= 60; tim += 10) {
		initialize(true, tim);

		fprintf(fp, "%d,%.6f,%.6f,%.6f,%.6f\n", tim, 
			cnt_test_ware(build_scm(cfg.memory)), cnt_test_ware(build_scu(cfg.memory)),
			cnt_test_ware(build_adacm(cfg.memory)), cnt_test_ware(build_iacm(cfg.memory)));
		fflush(fp);
	}
	fclose(fp);
}

void figure9_3() {
	FILE *fp = fopen("figure9_3.csv", "w");
	cfg = config("CAIDA", 8, 8, 60, MB(60));
	fprintf(fp, "Window Time(s),SCM,SCU,Ada CM,IACM\n");
	for (int tim = 10; tim <= 60; tim += 10) {
		initialize(true, tim);

		fprintf(fp, "%d,%.6f,%.6f,%.6f,%.6f\n", tim, 
			cnt_test_waae(build_scm(cfg.memory)), cnt_test_waae(build_scu(cfg.memory)),
			cnt_test_waae(build_adacm(cfg.memory)), cnt_test_waae(build_iacm(cfg.memory)));
		fflush(fp);
	}
	fclose(fp);
}

void figure10_1() {
	cfg = config("CAIDA", 8, 8, 60, MB(15));
	
	initialize();
	FILE *fp = fopen("figure10_1.csv", "w");
	double *fpr = new double[cfg.ds_win_num + 1];
	fprintf(stderr, "read complete\n");

	fprintf(fp, "query length,");
	for (int i = 1; i <= cfg.ds_win_num; ++i)
		fprintf(fp, "%d,", i);
	fprintf(fp, "\n");

	bf_test_multi_fpr(build_sbf(cfg.memory), fpr);
	output_line("sbf", fpr, fp);

	bf_test_multi_fpr(build_pbf(cfg.memory), fpr);
	output_line("pbf", fpr, fp);
	
	bf_test_multi_fpr(build_iabf(cfg.memory), fpr);
	output_line("iabf", fpr, fp);

	fclose(fp);
}

void figure10_2() {
	cfg = config("CAIDA", 8, 8, 60, MB(100));

	initialize(true);
	FILE *fp = fopen("figure10_2.csv", "w");
	double *are = new double[cfg.win_num + 1];

	fprintf(fp, "window id,");
	for (int i = 1; i <= cfg.ds_win_num; ++i)
		fprintf(fp, "%d,", i);
	fprintf(fp, "\n");

	fprintf(stderr, "Testing Stair CM\n");
	cnt_test_multi_are(build_scm(cfg.memory), are);
	output_line("scm", are, fp);

	fprintf(stderr, "Testing Stair CU\n");
	cnt_test_multi_are(build_scu(cfg.memory), are);
	output_line("scu", are, fp);

	fprintf(stderr, "Testing Ada CM\n");
	cnt_test_multi_are(build_adacm(cfg.memory), are);
	output_line("adacm", are, fp);

	fprintf(stderr, "Testing IACM\n");
	cnt_test_multi_are(build_iacm(cfg.memory), are);
	output_line("iacm", are, fp);
	fclose(fp);
}

void figure10_3() {
	cfg = config("CAIDA", 8, 8, 60, MB(100));

	initialize(true);
	FILE *fp = fopen("figure10_3.csv", "w");
	double *are = new double[cfg.win_num + 1];

	fprintf(fp, "window id,");
	for (int i = 1; i <= cfg.ds_win_num; ++i)
		fprintf(fp, "%d,", i);
	fprintf(fp, "\n");

	fprintf(stderr, "Testing Stair CM\n");
	cnt_test_multi_aae(build_scm(cfg.memory), are);
	output_line("scm", are, fp);

	fprintf(stderr, "Testing Stair CU\n");
	cnt_test_multi_aae(build_scu(cfg.memory), are);
	output_line("scu", are, fp);

	fprintf(stderr, "Testing Ada CM\n");
	cnt_test_multi_aae(build_adacm(cfg.memory), are);
	output_line("adacm", are, fp);

	fprintf(stderr, "Testing IACM\n");
	cnt_test_multi_aae(build_iacm(cfg.memory), are);
	output_line("iacm", are, fp);
	fclose(fp);
}

void figure11_1() {
	cfg = config("CAIDA", 32, 32, 30, 15*1024*1024);
	
	initialize(true);
	FILE *fp = fopen("figure11_1.csv", "w");
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

void figure11_2() {
	cfg = config("CAIDA", 32, 32, 1, 120*1024*1024);
	initialize(true);
	FILE *fp = fopen("figure11_2.csv", "w");
	
	double *qcnt = new double[cfg.win_num + 1];

	fprintf(fp, "window id,");
	for (int i = 1; i <= cfg.ds_win_num; ++i)
		fprintf(fp, "%d,", i);
	fprintf(fp, "\n");

	cnt_test_qcnt(build_scm(cfg.memory,5), qcnt);
	output_line("scm", qcnt, fp);

	cnt_test_qcnt(build_scu(cfg.memory,5), qcnt);
	output_line("scu", qcnt, fp);

	cnt_test_qcnt(build_adacm(cfg.memory), qcnt);
	output_line("adacm", qcnt, fp);

	cnt_test_qcnt(build_iacm(cfg.memory), qcnt);
	output_line("iacm", qcnt, fp);
	fclose(fp);
}

int main() {
	srand(1214);
	
	// figure5_1();
	// figure8_2();
	figure6_1();
	return 0;
}
