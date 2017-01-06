exec(open('../../../src/scope_manager.py', 'r').read())
a = [0] * 10
parity = [0] * 10

s_context()
i = 0
while (i < 10):
    a[i] = i
    s_context()
    if ((a[i] / 2) == 0):
        parity[i] = True
    else:
        parity[i] = False
    r_context()
    i = (i + 1)
r_context()
