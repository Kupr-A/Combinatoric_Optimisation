#include <bits/stdc++.h>
using namespace std;

struct Item {
    int cost;
    int weight;
    int index;
    double costPerWeight;

    Item(int c, int w, int i) : cost(c), weight(w), index(i) {
        costPerWeight = (double)cost / weight;
    }

    bool operator<(const Item& other) const {
        return costPerWeight > other.costPerWeight;  
    }
};

int N, W;
vector<Item> items;

int bestCost = 0;
vector<int> bestIndices;
chrono::steady_clock::time_point startTime;

bool isTimeExceeded(int ms = 950) {
    auto now = chrono::steady_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(now - startTime).count() > ms;
}

double calculateUpperBound(int idx, int currWeight, int currCost) {
    double bound = currCost;
    int remain = W - currWeight;
    for (int i = idx; i < N && remain > 0; i++) {
        if (items[i].weight <= remain) {
            remain -= items[i].weight;
            bound += items[i].cost;
        } else {
            bound += items[i].costPerWeight * remain;
            break;
        }
    }
    return bound;
}

void updateBestSolution(int currCost, const vector<int>& currItems) {
    if (currCost > bestCost) {
        bestCost = currCost;
        bestIndices = currItems;
    }
}


void branchAndBound(int idx, int currWeight, int currCost, vector<int>& currItems) {
    if (currWeight > W || isTimeExceeded()) return;

    updateBestSolution(currCost, currItems);

    if (idx == N) return;

    double bound = calculateUpperBound(idx, currWeight, currCost);
    if (bound <= bestCost) return;
    currItems.push_back(items[idx].index);
    branchAndBound(idx + 1, currWeight + items[idx].weight, currCost + items[idx].cost, currItems);
    currItems.pop_back();

    branchAndBound(idx + 1, currWeight, currCost, currItems);
}

void takeItem(int idx, int currWeight, int currCost, vector<int>& currItems) {
    currItems.push_back(items[idx].index);
    branchAndBound(idx + 1, currWeight + items[idx].weight, currCost + items[idx].cost, currItems);
    currItems.pop_back();
}

void skipItem(int idx, int currWeight, int currCost, vector<int>& currItems) {
    branchAndBound(idx + 1, currWeight, currCost, currItems);
}

void sortItems() {
    sort(items.begin(), items.end());
}

void greedyInit() {
    int totalWeight = 0;
    int totalCost = 0;
    vector<int> chosen;

    for (auto& it : items) {
        if (totalWeight + it.weight <= W) {
            totalWeight += it.weight;
            totalCost += it.cost;
            chosen.push_back(it.index);
        }
    }

    bestCost = totalCost;
    bestIndices = chosen;
}

void solve() {
    sortItems();
    greedyInit();

    vector<int> current;
    branchAndBound(0, 0, 0, current);
}

void readInput() {
    cin >> N >> W;
    items.reserve(N);
    for (int i = 0; i < N; i++) {
        int c, w;
        cin >> c >> w;
        items.emplace_back(c, w, i + 1);
    }
}

void printResult() {
    cout << bestCost << '\n';
    for (int idx : bestIndices) {
        cout << idx << ' ';
    }
    cout << '\n';
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    readInput();
    startTime = chrono::steady_clock::now();

    solve();
    printResult();

    return 0;
}
