#include "glpk.h"
#include <iostream>
#include <vector>
#include <tuple>
#include <map>

using namespace std;

int n, m;
vector<tuple<int, int, int>> edges;
map<pair<int, int>, int> edge_idx;
vector<int> x_vars;

void read_edges() {
    cin >> n >> m;
    edges.resize(m);

    for (int i = 0; i < m; ++i) {
        int u, v, w;
        cin >> u >> v >> w;
        if (u > v) swap(u, v);
        edges[i] = {u, v, w};
        edge_idx[{u, v}] = i;
    }
}

void setup_variables(glp_prob *lp) {
    glp_add_cols(lp, m);
    x_vars.resize(m);

    for (int i = 0; i < m; ++i) {
        int idx = i + 1;
        x_vars[i] = idx;
        glp_set_col_kind(lp, idx, GLP_BV);
        glp_set_obj_coef(lp, idx, get<2>(edges[i]));
    }
}

void setup_constraints(glp_prob *lp) {
    glp_add_rows(lp, n);
    for (int v = 0; v < n; ++v) {
        vector<int> ind = {0};
        vector<double> val = {0.0};
        for (int i = 0; i < m; ++i) {
            auto [u, w, _] = edges[i];
            if (u == v || w == v) {
                ind.push_back(x_vars[i]);
                val.push_back(1.0);
            }
        }
        glp_set_mat_row(lp, v + 1, ind.size() - 1, ind.data(), val.data());
        glp_set_row_bnds(lp, v + 1, GLP_FX, 1.0, 1.0);
    }
}

int solve_problem(glp_prob *lp) {
    glp_iocp parm;
    glp_init_iocp(&parm);

    glp_term_out(GLP_OFF);
    parm.presolve = GLP_ON;
    parm.mip_gap = 0.0;
    parm.cb_func = nullptr;
    parm.gmi_cuts = GLP_ON;  
    parm.cov_cuts = GLP_ON;    
    parm.clq_cuts = GLP_ON; 
    parm.br_tech = GLP_BR_MFV; 
    parm.bt_tech = GLP_BT_BFS; 

    glp_intopt(lp, &parm);
    return glp_mip_obj_val(lp);
}

vector<int> get_result(glp_prob *lp) {
    vector<int> result;
    for (int i = 0; i < m; ++i) {
        int col = x_vars[i];
        if (glp_mip_col_val(lp, col) > 0.5)
            result.push_back(i);
    }
    return result;
}

void output_result(int total_weight, const vector<int>& result) {
    cout << total_weight << '\n';
    for (size_t i = 0; i < result.size(); ++i) {
        cout << result[i];
        if (i + 1 < result.size()) cout << ' ';
    }
    cout << '\n';
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    read_edges();

    glp_prob *lp = glp_create_prob();
    glp_set_obj_dir(lp, GLP_MIN);

    setup_variables(lp);
    setup_constraints(lp);

    int total_weight = solve_problem(lp);
    vector<int> result = get_result(lp);
    output_result(total_weight, result);

    glp_delete_prob(lp);
    return 0;
}