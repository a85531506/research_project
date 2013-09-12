#!/usr/bin/python2 

import xml.etree.ElementTree as et
import sys

if len(sys.argv) != 3:
    sys.stderr.write("Usage: graphml2topo input.graphml output.topo\n")
    exit()

in_file = sys.argv[1] 
out_file = sys.argv[2]

root = et.parse(in_file)

nodes = set()
edges = set()

xmlns = "{http://graphml.graphdrawing.org/xmlns}"

for graph in root.iter(xmlns + 'graph'):
    for node in graph.iter(xmlns + 'node'):
        nodes.add( node.get('id') )
    for edge in graph.iter(xmlns + 'edge'):
        edges.add( (edge.get('source'), edge.get('target')) )

with open(out_file, 'w') as topo:
    topo.write("NUM-NODES: %d\n" % len(nodes))
    topo.writelines(["LINK: %s %s CC 1\n" % (edge[0], edge[1]) for edge in edges])
