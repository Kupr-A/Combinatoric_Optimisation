#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <stdexcept>
#include <chrono>
#include "glpk.h"
#include "algorithm"

using namespace std;
using namespace std::chrono;

void readInput(const string& file, int& n, int& m,
               vector<double>& cap, vector<double>& openC,
               vector<double>& dem, vector<vector<double>>& cost) {
    ifstream fin(file);
    if (!fin.is_open())
        throw runtime_error("error opening: " + file);

    fin >> n >> m;
    cap.resize(n);
    openC.resize(n);
    for (int i = 0; i < n; ++i)
        fin >> cap[i] >> openC[i];

    dem.resize(m);
    for (int i = 0; i < m; ++i)
        fin >> dem[i];

    cost.assign(n, vector<double>(m));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            fin >> cost[i][j];
}

void initVars(glp_prob* lp, int n, int m, const vector<double>& openC, const vector<vector<double>>& cost) {
    int vars = n * m + n;
    glp_add_cols(lp, vars);

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j) {
            int idx = i * m + j + 1;
            glp_set_col_bnds(lp, idx, GLP_DB, 0.0, 1.0);
            glp_set_obj_coef(lp, idx, cost[i][j]);
        }

    for (int i = 0; i < n; ++i) {
        int idx = n * m + i + 1;
        glp_set_col_kind(lp, idx, GLP_BV);
        glp_set_col_bnds(lp, idx, GLP_DB, 0.0, 1.0);
        glp_set_obj_coef(lp, idx, openC[i]);
    }
}

int addDemandConstraints(glp_prob* lp, int row, int n, int m, const vector<double>& dem) {
    vector<int> ind(n + 1);
    vector<double> val(n + 1);

    for (int j = 0; j < m; ++j) {
        for (int i = 0; i < n; ++i) {
            ind[i + 1] = i * m + j + 1;
            val[i + 1] = 1.0;
        }
        glp_set_row_bnds(lp, row, GLP_FX, 1.0, 1.0);
        glp_set_mat_row(lp, row++, n, ind.data(), val.data());
    }
    return row;
}

int addCapacityConstraints(glp_prob* lp, int row, int n, int m, const vector<double>& cap, const vector<double>& dem) {
    vector<int> ind(m + 2);
    vector<double> val(m + 2);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            ind[j + 1] = i * m + j + 1;
            val[j + 1] = dem[j];
        }
        ind[m + 1] = n * m + i + 1;
        val[m + 1] = -cap[i];

        glp_set_row_bnds(lp, row, GLP_UP, 0.0, 0.0);
        glp_set_mat_row(lp, row++, m + 1, ind.data(), val.data());
    }
    return row;
}

void printResult(glp_prob* lp, int n, int m) {
    vector<int> opened;
    for (int i = 0; i < n; ++i) {
        int idx = n * m + i + 1;
        if (glp_mip_col_val(lp, idx) > 0.5)
            opened.push_back(i + 1);
    }

    sort(opened.begin(), opened.end());

    cout << opened.size() << "\n";
    for (size_t i = 0; i < opened.size(); ++i)
        cout << opened[i] << (i + 1 == opened.size() ? "\n" : " ");

    cout << fixed << setprecision(6);
    for (int i : opened) {
        for (int j = 0; j < m; ++j) {
            int idx = (i - 1) * m + j + 1;
            double val = glp_mip_col_val(lp, idx);
            cout << (val < 1e-6 ? 0.0 : val) << (j + 1 == m ? "\n" : " ");
        }
    }
}

int main() {
    int n, m;
    vector<double> cap, openC, dem;
    vector<vector<double>> cost;

    try {
        readInput("input.txt", n, m, cap, openC, dem, cost);
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    auto start = high_resolution_clock::now();

    glp_prob* lp = glp_create_prob();
    glp_set_prob_name(lp, "facility");
    glp_set_obj_dir(lp, GLP_MIN);

    initVars(lp, n, m, openC, cost);
    glp_add_rows(lp, n + m);

    int row = 1;
    row = addDemandConstraints(lp, row, n, m, dem);
    row = addCapacityConstraints(lp, row, n, m, cap, dem);

    glp_iocp param;
    glp_init_iocp(&param);
    param.presolve = GLP_ON;
    param.gmi_cuts = GLP_ON;
    param.cov_cuts = GLP_ON;
    param.clq_cuts = GLP_ON;
    param.mir_cuts = GLP_ON;
    param.tol_int = 1e-4;
    param.tol_obj = 1e-4;
    param.msg_lev = GLP_MSG_OFF;

    glp_term_out(GLP_OFF);
    glp_intopt(lp, &param);

    auto stop = high_resolution_clock::now();
    printResult(lp, n, m);

    auto duration = duration_cast<milliseconds>(stop - start);
    // cerr << "Solved in " << duration.count() << " ms\n";

    glp_delete_prob(lp);
    return 0;
}
