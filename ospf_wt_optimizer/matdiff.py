#!/usr/bin/python2

import sys
if len(sys.argv) != 4:
    sys.stderr.write("Usage: matdiff old new changes")
    exit()

old_file = sys.argv[1]
new_file = sys.argv[2]
change_file = sys.argv[3]

def blank_mat(size):
    m = [[0 for _ in range(0, size)] for _2 in range(0, size)]
    return m

def parse_mat(s):
    rows = s.splitlines()
    mat_size = len(rows)
    mat = blank_mat(mat_size)

    for r, row in enumerate(rows):
        cells = row.split()
        for c, cell in enumerate(cells):
            mat[r][c] = int(cell)

    return mat

def make_diff(old, new):

    def key(r, c):
        return chr(ord('A') + r) + chr(ord('A') + c)

    diff = {}
    for r, row in enumerate(old):
        for c, cell in enumerate(row):
            if old[r][c] != new[r][c]:
                diff[ key(r, c) ] = new[r][c]

    return diff


f_old = open(old_file, 'r')
f_new = open(new_file, 'r')
old_mat = parse_mat(f_old.read())
new_mat = parse_mat(f_new.read())
f_old.close()
f_new.close()

diff = make_diff(old_mat, new_mat)

f_change = open(change_file, 'w')
f_change.write(repr(diff))
f_change.close()
