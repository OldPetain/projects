s = input().strip()
v = {}
for i in range(len(s)):
    if s[i] == ':':
        v['-'+s[i-1]] = 1
    else:
        v['-'+s[i]] = 0
n = int(input())
for i in range(n):
    result = 'Case ' + str(i+1) + ':'
    # 读入一行命名
    a = input().rstrip().split()
    arg = {}
    j = 1
    while j != len(a):
        if a[j] not in v:
            break 
        if v[a[j]] == 1:
            if (j == len(a)-1):
                break
            arg[a[j]] = a[j+1] 
            j = j+1
        else:
            arg[a[j]] = ''
        j = j+1
    if arg == {}:
        print(result + ' ')
    else:
        keys = list(arg.keys())
        keys.sort()
        for key in keys:
            result = result + ' ' + key
            if (arg[key]!= ''):
                result = result + ' ' + arg[key]
        print(result)