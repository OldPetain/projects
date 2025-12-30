from snode import Snode as Sn

class Slinkedlist:
    def __init__(self):
        self.head = None
        self.length = 0
        
    def append(self, data):
        node = Sn(data)
        if self.head != None:
            pl = self.head
            while pl.next !=None:
                pl = pl.next
            pl.next = node
        else:
            self.head = node
        self.length += 1
        
    def display(self):
        lst = []
        node = self.head
        while node != None:
            lst.append(node.data)
            node = node.next
        return lst
