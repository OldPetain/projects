n,m = map(int,input().split())
matrix = []
for i in range(n):
    matrix.append(list(map(int,input().split())))
for i in range(m-1,-1,-1): # range(m-1,-1,-1)表示从m-1到0，步长为-1
    for j in range(n):
        print(matrix[j][i],end = " ")
    print() # 每打印完一行换行
