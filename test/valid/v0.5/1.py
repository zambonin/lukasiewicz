exec(open('../../../src/scope_manager.py', 'r').read())

j = 0
s_context()
while (j < 10):
    j = (j + 2)
r_context()
s_context()
i = 0
while (i < 10):
    
    temp = (j + i)
    j = temp
    i = (i + 1)
r_context()
j = (j + 0)
