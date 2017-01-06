exec(open('../../../src/scope_manager.py', 'r').read())
a = 0
b = 1


teste_falso = False
s_context()
if (a > b):
    s_context()
    if (a > 0):
        c = 10
    r_context()
r_context()
s_context()
if teste_falso:
    d = 0
else:
    d = 20
r_context()
