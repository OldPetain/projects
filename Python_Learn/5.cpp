//动态分配CPU和GPU
#include <algorithm>
#include <cstdio>
#include <cstring>
#define MAXN 45
int n;
int a[MAXN],b[MAXN],c[MAXN],d[MAXN];
int dp[MAXN][MAXN*12];

int main() {
    int i,j;
    scanf("%d",&n);
    for(i=1;i<=n;i++) {
        scanf("%d%d%d%d",&a[i],&b[i],&c[i],&d[i]); //a:1个CPU，b:2个CPU，c:1个GPU加1个CPU，d:2个CPU加1个GPU
        if (c[i]>a[i])c[i] = a[i]; 
        if (d[i]>b[i])d[i] = b[i];
        if (d[i]>c[i])d[i] = c[i];
    }
    memset(dp,0x3f,sizeof(dp));
    dp[0][0] = 0;
    for(i=1;i<=n;i++) {
        for(j=0;j<=MAXN * 10;++j) {
            dp[i][j] = dp[i-1][j]+a[i];
            if (j>=c[i] && dp[i-1][j-c[i]] < dp[i][j])
                dp[i][j] = dp[i-1][j-c[i]];
            if (j>=d[i] && dp[i-1][j-d[i]] + d[i] < dp[i][j])
                dp[i][j] = dp[i-1][j-d[i]] + d[i];
        }
    }
    int ans = 1e9;
    for(j=0;j<=n*10;++j) {
        if(ans > dp[n][j] && ans > j) ans = std::max(dp[n][j],j);
    }
    printf("%d\n",ans);
    return 0;
}