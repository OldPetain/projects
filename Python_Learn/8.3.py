class node:
    def __init__(self, ak, ht):
        self.ak = ak
        self.ht = ht

v0 = [node(0,30)]
v1 = [node(0,30)]

n = int(input())
flag = 0  # 0表示先手，1表示后手

for _ in range(n):
    op = input().strip() #根据题意，输入的操作不需要分割
    if op.startswith('summon'):
        pos,ak,ht = map(int,op.split()[1:])
        if flag == 0:
            v0.insert(pos,node(ak,ht))
        else:
            v1.insert(pos,node(ak,ht))
    elif op.startswith('attack'):
        x,y = map(int,op.split()[1:])
        if flag == 0:
            v0[x].ht -= v1[y].ak
            v1[y].ht -= v0[x].ak
            if v0[x].ht <= 0 and x != 0:
                v0.pop(x)
            if v1[y].ht <= 0 and y != 0:
                v1.pop(y)
        else:
            v1[x].ht -= v0[y].ak
            v0[y].ht -= v1[x].ak
            if v1[x].ht <= 0 and x != 0:
                v1.pop(x)
            if v0[y].ht <= 0 and y != 0:
                v0.pop(y)
    elif op.startswith('end'):
        flag = 1 - flag #切换先后手

if v0[0].ht > 0 and v1[0].ht <= 0:
    print(1)
elif v0[0].ht <= 0 and v1[0].ht > 0:
    print(-1)
else:
    print(0)
    
# 输出先手玩家英雄生命值和随从数量
print(v0[0].ht)
print(len(v0) - 1, end=' ')
if len(v0) > 1:
    print(' '.join(str(v0[i].ht) for i in range(1, len(v0))))
else:
    print()

# 输出后手玩家英雄生命值和随从数量
print(v1[0].ht)
print(len(v1) - 1, end=' ')
if len(v1) > 1:
    print(' '.join(str(v1[i].ht) for i in range(1, len(v1))))
else:
    print()



        
        
    