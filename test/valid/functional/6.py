exec(open('../../../src/scope_manager.py', 'r').read())
t = [0] * 10
output = [0] * 10
def t_filter(t):
    def λ(x):
        return (x > 10)

        
    t_ta = [0] * 0
    s_context()
    t_ti = 0
    while (t_ti < len(t)):
        s_context()
        if λ(t[t_ti]):
            t_ta + [t[t_ti]]
        r_context()
        t_ti = (t_ti + 1)
    r_context()
    return t_ta

output = t_filter(t)
