exec(open('../../../src/scope_manager.py', 'r').read())




a = 2
b = 4
def λ(x, y):
  return (x + y)

c = λ(a, b)
def λ(x, y):
  return (x - y)

d = λ(a, b)
