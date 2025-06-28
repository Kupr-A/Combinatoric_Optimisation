#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef pair<ll, ll> pll;

struct Subset {
    ll weight, value;
    int mask;
};

void get_subsets(const vector<pll>& items, vector<Subset>& subsets) {
    int n = items.size();
    for (int mask = 0; mask < (1 << n); ++mask) {
        ll weight = 0, value = 0;
        for (int i = 0; i < n; ++i) {
            if (mask & (1 << i)) {
                weight += items[i].first;
                value += items[i].second;
            }
        }
        subsets.push_back({weight, value, mask});
    }
}

vector<Subset> optimize_subsets(vector<Subset>& subsets) {
    sort(subsets.begin(), subsets.end(), [](const Subset& a, const Subset& b) {
        return a.weight < b.weight;
    });
    vector<Subset> optimized;
    for (const auto& s : subsets) {
        if (optimized.empty() || s.value > optimized.back().value) {
            optimized.push_back(s);
        }
    }
    return optimized;
}

pair<int, int> find_best_combination(const vector<Subset>& left_subsets, const vector<Subset>& right_optimized, ll W) {
    ll best_value = 0;
    int best_maskL = 0, best_maskR = 0;
    for (const auto& left_set : left_subsets) {
        if (left_set.weight > W) continue;
        auto it = upper_bound(right_optimized.begin(), right_optimized.end(), Subset{W - left_set.weight, LLONG_MAX, 0}, 
            [](const Subset& a, const Subset& b) { return a.weight < b.weight; }) - 1;
        if (it >= right_optimized.begin()) {
            ll total_value = left_set.value + it->value;
            if (total_value > best_value) {
                best_value = total_value;
                best_maskL = left_set.mask;
                best_maskR = it->mask;
            }
        }
    }
    return {best_maskL, best_maskR};
}

vector<int> extract_items(int mask, int offset, int count) {
    vector<int> selected;
    for (int i = 0; i < count; ++i) {
        if (mask & (1 << i)) selected.push_back(offset + i + 1);
    }
    return selected;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    ll W;
    cin >> n >> W;
    
    vector<pll> items(n);
    for (int i = 0; i < n; ++i) {
        cin >> items[i].first >> items[i].second;
    }

    int mid = n / 2;
    vector<pll> left(items.begin(), items.begin() + mid);
    vector<pll> right(items.begin() + mid, items.end());

    vector<Subset> left_subsets, right_subsets;
    get_subsets(left, left_subsets);
    get_subsets(right, right_subsets);

    vector<Subset> right_optimized = optimize_subsets(right_subsets);
    auto [best_maskL, best_maskR] = find_best_combination(left_subsets, right_optimized, W);

    vector<int> selected = extract_items(best_maskL, 0, mid);
    vector<int> right_selected = extract_items(best_maskR, mid, n - mid);
    selected.insert(selected.end(), right_selected.begin(), right_selected.end());

    cout << selected.size() << '\n';
    for (int i : selected) cout << i << ' ';
    cout << '\n';

    return 0;
}
