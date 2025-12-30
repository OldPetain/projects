# include <algorithm>
# include <cmath>
# include <cstdio>
# include <cstdlib>
# include <cstring>
# include <iostream>
# include <queue>
using namespace std;

const int maxn = 200 + 10, maxm = 40000 + 10, S = 1,T=2;
int n , m ,k;
int dis[maxn][maxn];
int cur , fir[maxn], ver[maxm << 1],nxt [maxm << 1];
double r;
bool vis[maxn];
struct node{
    double x,y;
    void read(){
        scanf("%lf%lf",&x,&y);
        return;
    }
} p[maxn << 1];
void Add(int u,int v){
    ver[++cur] = v, nxt[cur] = fir[u], fir[u] = cur;
    ver[++cur] = u, nxt[cur] = fir[v], fir[v] = cur;
    return;
}
double Dis(node a,node b){
    double x = a.x - b.x, y = a.y - b.y;
    return sqrt(x * x + y * y);
}
void Work(){
    memset(dis,-1,sizeof dis);
    memset(vis,false,sizeof vis);
    queue<int> q;
    q.push(S);
    dis[S][0] = 0,vis[S] = true;
    while(!q.empty()){
        int u = q.front();
        for(int h = 0;h <= k;h++)
            if(dis[u][h] != -1)
                for(int i = fir[u] , v;i;i = nxt[i]){
                    v = ver[i];
                    if(v > n && h < k && (dis[v][h+1] == -1 || dis[v][h+1] > dis[u][h] + 1)){
                        dis[v][h+1] = dis[u][h] + 1;
                        if(!vis[v]){
                            q.push(v);
                            vis[v] = true;
                        }
                    } else if (dis[v][h] == -1 || dis[v][h] > dis[u][h] + 1){
                        dis[v][h] = dis[u][h] + 1;
                        if(!vis[v]){
                            q.push(v);
                            vis[v] = true;
                        }
                    }
                }
        q.pop();
    }
    int ans = -1;
    for(int i = 0 ; i <= k ; i ++ ){
        if (dis[T][i] != -1)
            ans = (ans == -1 ? dis[T][i] : min(ans,dis[T][i]));
    }
    printf("%d\n",ans -1);
    return;
}
int main(){
    while(scanf("%d%d%d%lf",&n,&m,&k,&r)!= EOF) {
        cur = 0;
        memset(fir,0,sizeof fir);
        for(int i = 1;i <= n + m;i++) {
            p[i].read();
        }
        for(int i = 1;i <= n + m;i++) {
            for(int j = i + 1;j <= n + m;j++) {
                if(Dis(p[i],p[j]) <= r) {
                    Add(i,j);
                }
            }
        }
        Work();
    }
    return 0;
}


