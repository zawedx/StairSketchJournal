#pragma once
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <cmath>
#include "common.hpp"

using namespace std;

double calc_topk_F1(pair<elem_t, int> *ground_truth_topk, pair<elem_t, int> *predicted_topk, int k = TOP_K){
    unordered_map<int, int> predicted_map;
    predicted_map.clear();
    for (int i = 0; i < k; i++){
        predicted_map[predicted_topk[i].first] = predicted_topk[i].second;
    }
    int tot = 0;
    double ARE = 0;
    double AAE = 0;
    int hit = 0;
    for (int i = 0; i < k; i++){
        int freq = predicted_map[ground_truth_topk[i].first];

        hit += (freq > 0);
        // tot++;
        tot += (ground_truth_topk[i].second > 0);
        if (freq != 0){
            ARE += fabs(ground_truth_topk[i].second - freq) / (double)ground_truth_topk[i].second;
            AAE += fabs(ground_truth_topk[i].second - freq);
        }
    }
    
    return (double)hit / (double)tot;
}


double calc_topk_ARE(pair<elem_t, int> *ground_truth_topk, pair<elem_t, int> *predicted_topk, int k = TOP_K){
    // unordered_map<int, int> ground_truth_map;
    unordered_map<int, int> predicted_map;
    predicted_map.clear();
    for (int i = 0; i < k; i++){
        // ground_truth_map[ground_truth_topk[i].first] = ground_truth_topk[i].second;
        predicted_map[predicted_topk[i].first] = predicted_topk[i].second;
    }
    int tot = 0;
    double ARE = 0;
    double AAE = 0;
    int hit = 0;
    for (int i = 0; i < k; i++){
        int freq = predicted_map[ground_truth_topk[i].first];

        hit += (freq > 0);
        // tot++;
        tot += (ground_truth_topk[i].second > 0);
        if (freq > 0){
            ARE += fabs(ground_truth_topk[i].second - freq) / (double)ground_truth_topk[i].second;
            AAE += fabs(ground_truth_topk[i].second - freq);
        } else {
            ARE += fabs(ground_truth_topk[i].second - 0) / (double)ground_truth_topk[i].second;
            AAE += fabs(ground_truth_topk[i].second - 0);
        }
    }
    
    // return (double)hit / (double)tot;
    return ARE / (double)tot;

    // avg_F1 += (double)hit / (double)tot;
    // avg_ARE += ARE / (double)hit;
    // avg_AAE += AAE / (double)hit;
}

double calc_KT(unordered_map<int, int>& ground_truth_map, unordered_map<int, int>& predicted_map){
    unordered_map<int, int>::iterator it, itx, ity;
    int P = 0, div = 0;

    // printf("_____[calc_KT]_____\n");
    // printf("display two map:\n");
    // for (it = ground_truth_map.begin(); it != ground_truth_map.end(); it++){
    //     printf("(%d, %d) ", it->first, it->second);
    // }
    // printf("\n");
    // for (it = predicted_map.begin(); it != predicted_map.end(); it++){
    //     printf("(%d, %d) ", it->first, it->second);
    // }
    // printf("\n");
    int count_predict = 0;
    for (itx = predicted_map.begin(); itx != predicted_map.end(); itx++){
        count_predict++;
        for (ity = predicted_map.begin(); ity != predicted_map.end(); ity++){
            // if (ground_truth_map[ity->first] == 0) continue;
            // if (predicted_map[itx->first] == 0) continue;
            int rel1, rel2;
            rel1 = ground_truth_map[itx->first] < ground_truth_map[ity->first] ? 1 : 0;
            rel2 = predicted_map[itx->first] < predicted_map[ity->first] ? 1 : 0;
            if (rel1 == rel2) ++P;
            ++div;
        }
    }
    printf("KT: predict size=%d\n", count_predict);

    // for (itx = ground_truth_map.begin(); itx != ground_truth_map.end(); itx++){
    //     for (ity = predicted_map.begin(); ity != predicted_map.end(); ity++){
    //         if (ground_truth_map[ity->first] == 0) continue;
    //         if (predicted_map[itx->first] == 0) continue;
    //         int rel1, rel2;
    //         rel1 = ground_truth_map[itx->first] < ground_truth_map[ity->first] ? 1 : 0;
    //         rel2 = predicted_map[itx->first] < predicted_map[ity->first] ? 1 : 0;
    //         if (rel1 == rel2) ++P;
    //         ++div;
    //     }
    // }
    // printf("P = %d, div = %d\n", P, div);
    double result = (double)P * 2 / div - 1;
    return result;
}

double calc_SRC(unordered_map<int, int>& ground_truth_map, unordered_map<int, int>& predicted_map){
    return 0;// wrong
    unordered_map<int, int>::iterator it, itx, ity;
    int sum = 0, n = 0;
    unordered_map<int, int> rank1, rank2;
    vector<pair<int, int> > arr1, arr2;

    for (it = ground_truth_map.begin(); it != ground_truth_map.end(); it++){
        arr1.push_back(make_pair(it->second, it->first));
        ++n;
    }
    sort(arr1.begin(), arr1.end(), greater<pair<int, int> >());
    for (it = predicted_map.begin(); it != predicted_map.end(); it++){
        arr2.push_back(make_pair(it->second, it->first));
    }
    sort(arr2.begin(), arr2.end(), greater<pair<int, int> >());
    
    // printf("_____[calc_SRC]_____\n");
    // printf("display two vector:\n");
    // for (int i=0; i<arr1.size(); i++){
    //     printf("(%d, %d) ", arr1[i].second, arr1[i].first);
    // }
    // printf("\n");
    // for (int i=0; i<arr2.size(); i++){
    //     printf("(%d, %d) ", arr2[i].second, arr2[i].first);
    // }
    // printf("\n");

    for (it = ground_truth_map.begin(); it != ground_truth_map.end(); it++){
        rank1[it->first] = lower_bound(arr1.begin(), arr1.end(), make_pair(it->second, it->first)) - arr1.begin();
    }
    for (it = predicted_map.begin(); it != predicted_map.end(); it++){
        rank2[it->first] = lower_bound(arr2.begin(), arr2.end(), make_pair(it->second, it->first)) - arr2.begin();
    }
    
    for (it = ground_truth_map.begin(); it != ground_truth_map.end(); it++){
        if (predicted_map[it->first] == 0) continue;
        int di = rank1[it->first] - rank2[it->first];
        sum += di * di;
    }
    return (double)sum * -6 / (n-1) / n / (n+1) + 1;
}

double calc_NDCG(unordered_map<int, int>& ground_truth_map, unordered_map<int, int>& predicted_map){
    unordered_map<int, int>::iterator it, itx, ity;
    double DCG = 0, IDCG = 0;
    unordered_map<int, int> rank1, rank2;
    vector<pair<int, int> > arr1, arr2;

    int count_predict = 0;
    for (it = predicted_map.begin(); it != predicted_map.end(); it++){
        ++count_predict;
        arr2.push_back(make_pair(it->second, it->first));
        arr1.push_back(make_pair(ground_truth_map[it->first], it->first));
    }
    printf("NDCG: predict size=%d\n", count_predict);
    sort(arr1.begin(), arr1.end(), greater<pair<int, int> >());
    sort(arr2.begin(), arr2.end(), greater<pair<int, int> >());

    for (int i=0; i<arr1.size(); i++){
        double rel = arr1[i].first;
        double div = log2(i+2);
        IDCG += rel / div;
    }
    for (int i=0; i<arr2.size(); i++){
        double rel = ground_truth_map[arr2[i].second];//arr2[i].first;
        double div = log2(i+2);
        DCG += rel / div;
    }
    printf("calc NDCG finish\n");
    return DCG / IDCG;
}

double calc_TAL(unordered_map<int, int>& ground_truth_map, unordered_map<int, int>& predicted_map){
    unordered_map<int, int>::iterator it, itx, ity;
    double DCG = 0, IDCG = 0;
    unordered_map<int, int> rank1, rank2;
    vector<pair<int, int> > arr1, arr2;

    int count_predict = 0;
    for (it = predicted_map.begin(); it != predicted_map.end(); it++){
        count_predict++;
        arr2.push_back(make_pair(it->second, it->first));
        arr1.push_back(make_pair(ground_truth_map[it->first], it->first));
    }
    printf("TAL: predict size=%d\n", count_predict);
    sort(arr1.begin(), arr1.end(), greater<pair<int, int> >());
    sort(arr2.begin(), arr2.end(), greater<pair<int, int> >());

    double sum = 0;
    double n = arr1.size();

    printf("n=%d\n", (int)n);
    for (int i=0; i<arr1.size(); i++){
        // printf("calc tal at i=%d\n", i);
        int j = 0;
        for (; j<arr2.size(); j++){
            if (arr2[j].second == arr1[i].second) break;
        }
        sum += ((double)i-j)*(i-j);
    }
    printf("calc tal finish\n");
    return 1-(6.0*sum/n/(n+1)/(n-1));
}