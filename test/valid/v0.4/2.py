exec(open('../../../src/scope_manager.py', 'r').read())

s_context()
if True:
    s_context()
    if (2.3 <= float(4)):
        s_context()
        if ((2 + 3) >= 5):
            f = 2.0
        r_context()
    else:
        f = 1.0
    r_context()
r_context()
