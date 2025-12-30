#include "chinese_postman.h"
#include <fstream>
#include <iostream>
#include <queue>
#include <limits>
#include <algorithm>
#include <unordered_map>
#include <stack>

using namespace std;

// 从文件加载图数据, 文件格式: n m, 随后 m 行: u v w (结点为1-based)
bool ChinesePostman::loadFromFile(const string &path) {
    ifstream ifs(path); // 打开文件
    if (!ifs) return false; // 文件打开失败
    edges.clear(); // 清空已有边数据

    // 从文件读取顶点数和边数
    int m; 
    ifs >> n >> m; 
    for (int i = 0; i < m; ++i) { 
        int u,v; double w; ifs >> u >> v >> w; 
        edges.push_back({u-1,v-1,w});
    }
    return true; 
}

// 从标准输入加载图数据, 交互式提示用户输入
bool ChinesePostman::loadFromStdin() {
    cout << "请输入顶点数 n 和 边数 m (空格分隔), 然后每行 u v w (1-based):\n"; // 提示用户输入
    edges.clear(); // 清空已有边数据

    // 从标准输入读取顶点数和边数
    if (!(cin >> n)) return false; 
    int m; if (!(cin >> m)) return false; 
    for (int i = 0; i < m; ++i) { 
        int u,v; double w; cin >> u >> v >> w; 
        edges.push_back({u-1,v-1,w}); 
    }
    return true;
}

static const double INF = 1e18;

// 判断图的连通性, 只考虑度>0 的顶点
bool ChinesePostman::isConnected(int start) const {
    if (n==0) return true; // 空图视为连通
    vector<vector<int>> adj(n); // 无向邻接表
    vector<int> deg(n,0); // 记录每个顶点的度数
    // 构造无向邻接表, 并统计度数 
    for (auto &e: edges) { 
        adj[e.u].push_back(e.v); // 无向图, 双向加入
        adj[e.v].push_back(e.u); 
        deg[e.u]++; // 增加度数
        deg[e.v]++; }
    int s = start; //start默认为0
    // 如果起点度为0, 则寻找第一个度>0 的顶点作为起点
    if (deg[s]==0) {
        s = -1; // 标记未找到
        for (int i=0;i<n;++i) if (deg[i]>0) { s=i; break; } // 找到第一个度>0 的顶点
        if (s==-1) return true; // 没有边的图视为连通
    }
    vector<char> vis(n,0); // 访问标记数组
    queue<int>q; q.push(s); vis[s]=1; // 标记起点已访问
    while(!q.empty()){ // BFS 遍历
        int v=q.front(); // 取出队首
        q.pop(); // 弹出队首
        for(auto to:adj[v]){ // 遍历邻接点
            if(!vis[to]){ // 未访问则标记并入队
                vis[to]=1;q.push(to); // 标记访问
            } 
        } 
    }
    for (int i=0;i<n;++i) if (deg[i]>0 && !vis[i]) return false; // 存在度>0 且未访问的顶点, 图不连通
    return true;
}

// 求解中国邮路问题的主入口, 参数 start 表示邮局所在的起点编号(0-based)
CPPResult ChinesePostman::solve(int start) {
    CPPResult res;  // 初始化结果
    res.total_cost = -1; // 默认不可解
    if (n<=0) return res; // 空图直接返回
    if (!isConnected(start)) return res; // 如果图在度>0 的顶点间不连通, 返回失败

    // 步骤1: 构造邻接矩阵并统计原有边的总权重
    double orig_sum = 0; // 记录原始边权之和
    vector<vector<double>> dist(n, vector<double>(n, INF)); // 初始化距离矩阵为无穷大
    vector<vector<int>> next(n, vector<int>(n, -1)); // 初始化路径下一跳矩阵
    vector<int> deg(n,0); // 记录每个顶点的度数
    for (int i=0;i<n;++i){ dist[i][i]=0; next[i][i]=i; } // 自身到自身距离为0， 下一跳为自身
    for (auto &e: edges) {
        orig_sum += e.w; // 记录原始边权之和
        deg[e.u]++; deg[e.v]++;
        // 如果有多重边, 保留最短的一条用于最短路计算
        if (e.w < dist[e.u][e.v]) {
            dist[e.u][e.v] = dist[e.v][e.u] = e.w;
            next[e.u][e.v] = e.v;
            next[e.v][e.u] = e.u;
        }
    }

    // 步骤2: 使用 Floyd-Warshall 计算任意两点之间的最短距离和路径下一跳
    for (int k=0;k<n;++k) {
        for (int i=0;i<n;++i) {
            if(dist[i][k]<INF) {
                for (int j=0;j<n;++j){
                    if (dist[k][j]<INF && dist[i][k]+dist[k][j] < dist[i][j]){
                        dist[i][j] = dist[i][k]+dist[k][j]; // 更新最短距离
                        next[i][j] = next[i][k];// 更新路径下一跳
                    }
                }
            }
        }
    }

    // 步骤3: 找出所有奇数度顶点, 这些顶点需要配对并加边
    vector<int> odd;
    for (int i=0;i<n;++i) if (deg[i]%2==1) odd.push_back(i);

    double added_cost = 0; // 记录加边的总权重
    vector<pair<int,int>> matching_pairs; // 记录最终的配对边

    // 如果存在奇数度顶点, 则使用位掩码DP求最小配对代价(最小权完全匹配)
    if (!odd.empty()) {
        int m = (int)odd.size();
        // 构造奇点之间的距离矩阵
        vector<vector<double>> od(m, vector<double>(m, 0)); // od[i][j] 表示 odd[i] 和 odd[j] 之间的最短距离
        for (int i=0;i<m;++i) for (int j=0;j<m;++j) od[i][j]=dist[odd[i]][odd[j]]; // 填充距离矩阵
        // DP over bitmask, dp[mask] 表示 mask 中已配对顶点的最小代价
        int FULL = 1<<m; // 全掩码
        vector<double> dp(FULL, INF); // 初始化为无穷大
        vector<int> choice(FULL, -1); // 记录选择的配对方案
        dp[0]=0; // 空集代价为0
        for (int mask=0; mask<FULL; ++mask) { // 遍历所有掩码
            if (dp[mask] >= INF) continue; // 不可达状态跳过
            int i=0; while(i<m && (mask & (1<<i))) ++i; // 找到第一个未配对顶点
            if (i>=m) continue; // 全部配对完成
            for (int j=i+1;j<m;++j) if (!(mask & (1<<j))) { // 尝试与 j 配对
                int nmask = mask | (1<<i) | (1<<j); // 新掩码
                double cost = dp[mask] + od[i][j]; // 计算配对代价
                if (cost < dp[nmask]) { dp[nmask]=cost; choice[nmask]= (i<<8) | j; } // 更新 DP 和选择
            }
        }
        added_cost = dp[FULL-1]; // 最小配对代价
        // 通过 choice 数组回溯出具体配对方案
        int cur = FULL-1; // 从全掩码开始回溯
        while (cur) { // 直到掩码为空
            int ch = choice[cur]; // 取出当前选择
            if (ch<0) break; // 不存在选择则跳出
            int i = (ch>>8)&0xff; int j = ch & 0xff; // 解码出配对顶点
            matching_pairs.push_back({odd[i], odd[j]}); // 记录配对边
            cur = cur & ~(1<<i) & ~(1<<j); // 更新掩码, 移除已配对顶点
        }
    }

    // 步骤4: 构造多重图, 将原边加入, 并沿着最短路径复制配对边对应的各段
    struct E2{int u,v; double w; bool used;};
    vector<E2> e2;
    vector<vector<pair<int,int>>> adj(n); // adj[u] 保存 (edge id, neighbor)
    auto addEdge = [&](int u,int v,double w){ int id = (int)e2.size(); e2.push_back({u,v,w,false}); adj[u].push_back({id,v}); adj[v].push_back({id,u}); };
    for (auto &e: edges) addEdge(e.u,e.v,e.w);
    // 对于每一对匹配顶点, 沿着 next 矩阵恢复最短路径并在多重图中复制这些边
    for (auto &p: matching_pairs) {
        int u = p.first, v = p.second;
        if (next[u][v]==-1) { res.total_cost=-1; return res; }
        int curv = u;
        while (curv != v) {
            int nx = next[curv][v];
            // 在多重图中添加一条表示复制的边(curv, nx)
            addEdge(curv, nx, dist[curv][nx]);
            curv = nx;
        }
    }

    double total = orig_sum + added_cost; // 最终总成本为原边和加边之和

    // 步骤5: 在构造的欧拉图上使用 Hierholzer 算法找欧拉回路
    vector<int> edge_pos(n,0); // 记录每个顶点当前使用的边位置
    stack<int> st; vector<int> circuit; // 存储欧拉回路
    st.push(start); // 从指定起点开始
    while(!st.empty()){
        int v = st.top(); // 当前顶点
        // 跳过已使用的边
        while (edge_pos[v] < (int)adj[v].size() && e2[adj[v][edge_pos[v]].first ].used) ++edge_pos[v];// 当前顶点的边已用完或已使用则跳过
        if (edge_pos[v] == (int)adj[v].size()) { circuit.push_back(v); st.pop(); } // 无可用边, 回溯
        else { 
            auto pr = adj[v][edge_pos[v]++]; int eid = pr.first; int to = pr.second; // 取出边和邻接点
            if (e2[eid].used) continue; // 已使用则跳过
            e2[eid].used = true; // 标记边为已使用
            st.push(to); // 继续前进
        }
    }
    reverse(circuit.begin(), circuit.end());

    res.total_cost = total;
    res.euler_path = move(circuit);
    return res;
}
