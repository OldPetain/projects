n = int(input())
a = list(map(int,input().split()))
a = a[:n]
b = {}
for i in a:
    b[i] = b.get(i,0) + 1

# 按值升序，值相同按键升序排序
for k, v in sorted(b.items(), key=lambda x: (-x[1], x[0])):
    print(f"{k}: {v}")