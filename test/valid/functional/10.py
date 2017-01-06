exec(open('../../../src/scope_manager.py', 'r').read())
t = [0] * 10
output = [0] * 10
def t_map(t):
    def λ(x):
        return x

        
    t_ta = [0] * 10
    s_context()
    t_ti = 0
    while (t_ti < len(t)):
        t_ta[t_ti] = λ(t[t_ti])
        t_ti = (t_ti + 1)
    r_context()
    return t_ta

output = t_map(t)
