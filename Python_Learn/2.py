class window:
    def __init__(self,x1,y1,x2,y2):
        self.x1 = x1        
        self.y1 = y1
        self.x2 = x2
        self.y2 = y2

def check(x,l,r):
    return x<=r and x>=l

def ask(N,tim,x,y):
    t = 0 
    for i in range(1,N+1):
        if check(x,w[i].x1,w[i].x2) and check(y,w[i].y1,w[i].y2) and rank[i] > rank[t]:
            t = i
    if t == 0:
        print("IGNORED")
    else:
        rank[t] = tim
        print(t)
    
N,M = map(int,input().split())
w = [None]*(N+1)
rank = [0]* (N+1)
for n in range(1,N+1):
    x1,y1,x2,y2 = map(int,input().split())
    w[n] = window(x1,y1,x2,y2)
    rank[n] = n

for m in range(1,M+1):
    x,y = map(int,input().split())
    ask(N,N+m,x,y)


    



    
    


'''
N,M = int(input().split())
a = []
b = []
for n in range(N):
    a[n] = list(map(int,input().split())) + n
    
for m in range(M):
    b[m] = list(map(int,input().split()))
    
for m in range(M):
    flag = 0 
    for n in range(N):
        if b[m][0] > a[n][0] and b[m][1] > a[n][1] and b[m][2] < a[n][2] and b[m][3] < a[n][3]:
            if (a[n][4] > a[])
            flag = 
'''