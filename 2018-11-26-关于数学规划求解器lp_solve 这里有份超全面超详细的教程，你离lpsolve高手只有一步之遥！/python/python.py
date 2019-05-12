>>> f = [4, 2, 1]
>>> A = [[2, 1, 0], [1, 0, 2], [1, 1, 1]]
>>> b = [1, 2, 1]
>>> l = [ 0, 0, 0]
>>> u = [ 1, 1, 2]


>>> from lpsolve55 import *
>>> from lp_maker import *
>>> lp = lp_maker(f, A, b, [-1, -1, 0], l, u, None, 1, 0)
>>> solvestat = lpsolve('solve', lp)
>>> obj = lpsolve('get_objective', lp)
>>> print obj
2.5
>>> x = lpsolve('get_variables', lp)[0]
>>> print x
[0.5, 0.0, 0.5]
>>> lpsolve('delete_lp', lp)