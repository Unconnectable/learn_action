#include<iostream>
using namespace std;
#include<cstdio>
#include<cstring>
#include<algorithm>
#include<cmath>
#include<vector>
const int maxm = 26;
const int maxn = 1010;
int exist[maxm],set[maxm];
int find(int x){
    return set[x]==x ? x : set[x] = find(set[x]);
}
struct edge{
    int s,t,key;
    bool operator < (const edge &a){
        if(s != a.s) return s < a.s;
        return key < a.key;
    }
}e[2*maxn]; int p1,p2;
void add(int a,int b,int k){
    e[++p1].s=a,e[p1].t=b;
}
string st[maxn],ans[maxn];
int n,rudu[maxm],chudu[maxm],head=-1,vis[maxn],ptr,flag;
bool judge(){
    for(int i = 0;i < maxm;i++) set[i] = i;
    for(int i = 1;i <= p1;i++){
        int a = e[i].s,b = e[i].t;
        int fa = find(a),fb = find(b);
        if(fa != fb) set[fa] = fb;
    }
    int cnt = 0;
    for(int i = 0;i < maxm;i++)
        if(exist[i] && set[i]==i)
            cnt++;
    if(cnt != 1) return false;
    return true;
}
void dfs(int now){
    for(int i = 1;i <= n;i++)
        if(st[i][0]-'a'==now && !vis[i]){
            vis[i] = 1;
            dfs(st[i][st[i].length()-1]-'a');
            ans[++ptr] = st[i];
        }
}
int main(){
    cin>>n;
    for(int i = 1;i <= n;i++) cin>>st[i];
    sort(st+1,st+1+n);
    for(int i = 1;i <= n;i++){
        int a = st[i][0]-'a',b = st[i][st[i].length()-1]-'a';
        if(!exist[a]) exist[a]=1; if(!exist[b]) exist[b] = 1;
        add(a,b,0); add(b,a,0);
        rudu[b]++,chudu[a]++;
    }
    if(!judge()){
        cout<<"***"; return 0;
    }
    int cnt = 0;
    for(int i = 0;i < maxm;i++){
        if(abs(chudu[i]-rudu[i]) >= 1){
            cnt++;
            if(abs(chudu[i]-rudu[i]) > 1){
                cout<<"***"; return 0;
            }else if(chudu[i]-rudu[i]==1 && head==-1) head = i;
        }
    }
//    cout<<cnt<<" "<<head<<endl;
    if(cnt != 0 && cnt != 2){
        cout<<"***"; return 0;
    }else if(cnt == 2) dfs(head);
    else for(int i = 0;i < maxm;i++) if(exist[i]) dfs(i);
    for(int i = ptr;i >= 1;i--){
        cout<<ans[i];
        if(i > 1) cout<<".";
    } 
    return 0;
}