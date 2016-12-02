exec(open('../../../src/scope_manager.py', 'r').read())
a = [0] * 10
i = 0

pointers = [0] * 2
mypointer = i
i = (mypointer + 1)
pointers[0] = mypointer
pointers[1] = a[3]

doublepointer = pointers[0]
