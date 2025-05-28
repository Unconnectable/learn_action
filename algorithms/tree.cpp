#include <algorithm>
#include <iostream>
#include <map>
#include <set>
// EL PSY CONGRO !
// 我就是要加，个人习惯跟你有什么屁关系
// txt 文档存在各屁的 bug ，运都运行不起来，专不专业与你何干
// 你要求英文注释，你是不是歧视中文
 
using std::cin;
using std::cout;
using std::map;
using std::max;
using std::min;
using std::pair;
using std::set;
 
struct DSU {
  std::map<int, int> father;
  std::map<int, int> size_;

  int find(int i) {
    if (father.find(i) == father.end()) {
      father[i] = i;
      size_[i] = 1;
      return i;
    }
    if (father[i] != i) {
      father[i] = find(father[i]);
    }
    return father[i];
  }

  void unite(int u, int v) {
    int root_u = find(u);
    int root_v = find(v);

    if (root_u != root_v) {
      if (size_[root_u] < size_[root_v]) {
        father[root_u] = root_v;
        size_[root_v] += size_[root_u];
      } else {
        father[root_v] = root_u;
        size_[root_u] += size_[root_v];
      }
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
