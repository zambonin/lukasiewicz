exec(open('../../../src/scope_manager.py', 'r').read())
values = [0] * 100
values[(int(values[10]) + 2)] = -values[(int(values[12]) * 5)]
