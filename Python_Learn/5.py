n = int(input().split()[0])
a = [0] * (n+1)
b = [0] * (n+1)
c = [0] * (n+1)
d = [0] * (n+1)
for i in range(1, n+1):
    a[i], b[i], c[i], d[i] = map(int, input().split())
    if c[i] > a[i]:
        c[i] = a[i]
    if d[i] > b[i]:
        d[i] = b[i]
    if d[i] > c[i]:
        d[i] = c[i]

dp = [[0] * 45 * 12 for _ in range(45)] 
dp[0][0] = 0
for i in range(1, n+1):
    for j in range(45):
        