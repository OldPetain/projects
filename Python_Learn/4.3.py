month = [[0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31],[0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]]

def leapyear(year):
    if (year % 4 == 0 and year % 100 != 0) or (year % 400 == 0):
        return 1
    else:
        return 0

a,b,c,y1,y2 = map(int, input().split())
d = 0
y = 0
days = 0
for i in range(1850,y1): # 1850年到y1年的总天数
    days += (365 + leapyear(i))
    
for i in range(y1,y2+1): # y1年到y2年的总天数
    y = leapyear(i)
    days2 = days
    for j in range(1,a):
        days2 += month[y][j]
    weekd = 1 + ((days2+1) % 7) # y年a月1号是星期几
    
    #if weekd >= c: # 1号是星期几大于等于星期c
    #    d = (b-1) * 7 + (c - weekd + 1)
    #else:
    #    d = (b-1) * 7 + (c - weekd + 1) #?
    d = (b-1) * 7 + (c - weekd + 1)
        
    if d > month[y][a] or d < 1: # 超过这个月的天数或小于1号
        print("none")
    else:
        print(i,end = '/')
        if a<10:
            print("0",end = '')
        print(a,end = '/')
        if d < 10:
            print("0",end = '')
        print(d)
    days += 365 + leapyear(i)