from slinkedlist import Slinkedlist as Sl

slinkedlist = Sl()
for ii in range(10):
    slinkedlist.append(ii)
lst = slinkedlist.display()
print(lst)
