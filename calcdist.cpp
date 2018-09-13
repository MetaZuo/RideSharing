#include <cstdio>
#include <cstdlib>
#include <vector>
#include "util.cpp"
using namespace std;

int main() {
    initialize(true);
    printf("load_end\n\n");
    map<pair<int, int>, int> *dist = new map<pair<int, int>, int>;
    vector<int> nodes;
    nodes.reserve(250000);
    nodes.clear();

    printf("reading requests\n");
    FILE *in = NULL;
    in = fopen("part_reqs.csv", "r");
    char buf[150];
    fgets(buf, 150, in);
    int num = 0;
    int start, end, reqTime;
    while (num != EOF) {
        num = fscanf(in, "%d,%d,%d\n", &reqTime, &start, &end);
        if (num != EOF) {
            nodes.push_back(start);
            nodes.push_back(end);
        }
    }
    fclose(in);
    printf("end reading\n\n");

    int max_vehicle = 400;

    printf("reading vehicles\n");
    in = fopen("vehicles.csv", "r");
    int loc;
    for (int i = 0; i < max_vehicle; i++) {
        fscanf(in, "%d\n", &loc);
        nodes.push_back(loc);
    }
    fclose(in);
    printf("end reading\n\n");

    printf("begin to calculate\n");
    int cnt = 0;
    for (int i = 0; i < nodes.size(); i++) {
        for (int j = i; j < nodes.size(); j++) {
            cnt++;
            get_dist(nodes[i], nodes[j], dist);
            if (cnt % 1000 == 0) {
                printf("%d\n", cnt);
            }
        }
    }
    printf("\ncalculation finished!\n\n");

    printf("begin to write file\n");
    FILE *out = NULL;
    out = fopen("dists.csv", "w");
    cnt = 0;
    int numPairs = dist->size();
    map<pair<int, int>, int>::iterator iter = dist->begin();
    while (iter != dist->end()) {
        cnt++;
        fprintf(out, "%d,%d,%d\n", iter->first.first, iter->first.second, iter->second);
        iter++;
        if (cnt % 1000 == 0) {
            printf("%d/%d\n", cnt, numPairs);
        }
    }
    fclose(out);
    printf("all finished!\n");

    return 0;
}