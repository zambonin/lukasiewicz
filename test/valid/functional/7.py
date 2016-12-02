exec(open('../../../src/scope_manager.py', 'r').read())
t = [0] * 10

def t_fold(t):
  def λ(x, y):
    return (x + y)

    
  t_tv = t[0]
  
  s_context()
  t_ti = 1
  while (t_ti < len(t)):
    t_tv = (t_tv + λ(t_tv, t[t_ti]))
    t_ti = (t_ti + 1)
  r_context()
  return t_tv

output = t_fold(t)
