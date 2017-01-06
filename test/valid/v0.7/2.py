exec(open('../../../src/scope_manager.py', 'r').read())
def fibo(x, b):
    
    s_context()
    if (x < 2):
        ans = x
    else:
        ans = (fibo((x - 1), b) + fibo((x - 2), b))
    r_context()
    return ans


a = fibo(10, True)
