exec(open('../../../src/scope_manager.py', 'r').read())
f = 1.0
g = 0.
h = .10

b = True
i = ((-f * g) - (h / 2.1))
b = ((not (i > 0.0)) | (i < -2.3))
