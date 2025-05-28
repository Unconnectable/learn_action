#include <algorithm>
#include <iostream>
#include <map>
#include <set>
 
using std::cin;
using std::cout;
using std::map;
using std::max;
using std::min;
using std::pair;
using std::set;
 
struct DSU {
  map<int, int> father;
  int find(int i) {
    if (father.find(i) == father.end()) {
      father[i] = i;
      return i;
    }
    if (father[i] == i)
      return i;
    else {
      // return father[i] = find(father[i]);
 
      while (i != father[i]) {
        i = father[i];
      }
      return i;
    }
  }
  void unite(int u, int v) {
    int father_u = find(u);
    int father_v = find(v);
    if (father_v != father_u) {
      father[v] = father_u;
    }
  }
};
 
int main() {
  int u, v;
  while (cin >> u >> v && (u != -1 and v != -1)) {
 
    // while (cin >> u >> v and (u != -1 and v != -1)){
    bool has_edge = false;
    set<int> nodes;
    set<pair<int, int>> edges;
    DSU dsu;
 
    while (u != 0 && v != 0) {
      nodes.insert(u);
      nodes.insert(v);
      /* has_edge = true;
      int a = min(u, v);
      int b = max(u, v);
      // 小->大
 
      edges.insert({a, b});
      dsu.unite(a, b); */
      // if (u != 0 && v != 0) {
      has_edge = true;
      int a = min(u, v);
      int b = max(u, v);
      edges.insert({a, b});
      // v的father是u father father
      dsu.unite(u, v);
      //}
      cin >> u >> v;
    }
 
    int E = edges.size();
    int V = nodes.size();
 
    bool ans = true;
    if (has_edge) {
      int root = dsu.find(*nodes.begin());
      for (auto node : nodes) {
        if (dsu.find(node) != root) {
          ans = false;
          break;
        }
      }
    } else {
      ans = false;
    }
 
    if (ans and E == V - 1) {
      cout << "Yes" << std::endl;
    } else {
      cout << "No" << std::endl;
    }
  }
 
  return 0;
}