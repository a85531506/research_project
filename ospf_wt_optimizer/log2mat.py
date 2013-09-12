#!/usr/bin/python2

import ast
import sys
if len(sys.argv) != 4:
    sys.stderr.write("Usage: log2mat logfile size matrix  -> will generate matrix.1, matrix.2, etc.")
    exit()

log_file = sys.argv[1]
size = 1 + int(sys.argv[2])
out_prefix = sys.argv[3]

def blank_mat():
    m = [["0" for _ in range(1, size)] for _2 in range(1, size)]
    return m

def ij_from_key(k):
    from_node = k[0]
    to_node = k[1]
    def asc2int(a):
        return ord(a) - ord('A')
    return (asc2int(from_node), asc2int(to_node))


def emit(line, i):
    with open(out_prefix + "." + str(i), 'w') as f:
        weights = ast.literal_eval(line.rstrip('\r\n'))
        m = blank_mat()
        for link, weight in weights.iteritems():
            (i, j) = ij_from_key(link)
            m[i][j] = str(weight)

        mat_str = ''
        for row in m:
            mat_str += '\t'.join(row) + '\n'

        f.write(mat_str)


i = 1
with open(log_file, 'r') as f:
    for line in f:
        if line[0] == '{':
            emit(line, i)
            i += 1

