#!/usr/bin/env python3


def get_row_tuples(instream):
    return (tuple(col.strip() for col in line.split(',')) for line in instream)


def read_nodes(tuples):
    return ((node[0], node[1], tuple(node[i:i+3] for i in range(2, len(node), 3)))
            for node in tuples)


def read_callgraph(instream):
    tuples = tuple(get_row_tuples(instream))
    delim  = tuples.index(('',))
    nodes  = tuple(read_nodes(tuples[:delim]))
    edges  = tuples[delim+1:]
    return (nodes, edges)


def print_callgraph(callgraph):
    nodes, edges = callgraph
    print('digraph {\n  node [shape=record];')

    for node in nodes:
        callsites = ''.join('|<l{0}>{1}:{2}'.format(*site) for site in node[2])
        node_format = '  {0}[label=\"{{{0}|Weight: {1}{2}}}\"];'
        print(node_format.format(node[0], node[1], callsites))

    for edge in edges:
        print('  {0}:l{1} -> {2};'.format(*edge))

    print('}')


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('csv', nargs='?', default=None,
                        help='the CSV format callgraph to transform')
    args = parser.parse_args()

    import sys
    with (open(args.csv) if args.csv else sys.stdin) as infile:
        callgraph = read_callgraph(infile)
    print_callgraph(callgraph)
