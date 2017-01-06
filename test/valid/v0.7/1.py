exec(open('../../../src/scope_manager.py', 'r').read())
def f():
    return False

s_context()
if f():
    a = 0
    def f2(x):
        
        a = (x + 1)
        return a

    a = f2(a)
r_context()
