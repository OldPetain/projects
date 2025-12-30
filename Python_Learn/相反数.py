n = int(input()) - 1
a = list(map(int,input().split()))
a = a[:n+1]
b = {}

ans = 0
for num in a:
    if num >= 0:
        b[num] = b.get(num,0) + 1
    
for num in a:
    if num < 0:
        ans += b.get(-num,0)
        
print(f"{ans}")

    