#include <cstdio>
#include <stack>
#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>
#define ep emplace_back 

#define ios std::ios::sync_with_stdio(false);std::cin.tie(0); 
const int N = 4e5+9;

using namespace std;

int n, m;
int head[N], idx = 0;
int time__;
int low[N], dfn[N];
struct node{
    int to, val, next;
} e[N<<2];
bool vis[N];
int scc[N], scc_cnt = 0;


void add(int u, int v, int val){
    e[idx] = {v, val, head[u]};
    head[u] = idx++;
}
int neighbor(int x){
    if(x%2==0) return x-1;
    return x+1;
}
void bd(){
    memset(head, -1, sizeof head);
    cin >> n >> m;
    for(int k = 1; k <= m; ++k){
        int i,j;
        cin>>i>>j;
        add(i , neighbor(j) , 0);       
        add(j , neighbor(i) , 0);
    }
}

stack<int> st;
void tarjan(int u){
    low[u] = dfn[u] = ++time__;
    st.push(u);
    vis[u] = 1;
    for(int i = head[u]; i != -1; i = e[i].next){
        int v = e[i].to;
        if(!dfn[v]){
            tarjan(v);
            low[u] = min(low[u], low[v]);
        }
        else if(vis[v]){
            low[u] = min(low[u], dfn[v]);
        }
    }
    if(dfn[u] == low[u]){
        scc_cnt++;
        while(true){
            int v = st.top();
            st.pop();
            vis[v] = 0;
            scc[v] = scc_cnt;
            if(u == v) break;
        }
    }
}
void solve(){
    bd();
    for(int i = 1; i <= 2 * n; ++i)
        if(!dfn[i])
            tarjan(i);
    bool ok = 1;
    for(int i = 1; i <= 2*n; i+=2){
        if(scc[i] == scc[neighbor(i)]){
            ok = 0;
            break;
        }
    }
    if(ok){
        for(int i=1 ; i<=2*n ; i+=2){
            cout<<(scc[i] < scc[i+1] ? i:i+1);
            cout<<"\n";
        }
    }
    else cout<<"NIE";
}

int main(){
    ios;
    solve();
    return 0;
}