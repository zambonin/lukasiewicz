exec(open('../../../src/scope_manager.py', 'r').read())
f = 0.0
g = 1.0
s_context()
while (f <= 12.3):
    f = (f + (g * g))
r_context()
