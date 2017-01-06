exec(open('../../../src/scope_manager.py', 'r').read())
i = 0

f = 1.1
b = True
j = int(int((float(i) + f)))
i = int(j)
b = (b & bool(f))
f = (float(b) + 0.0)
