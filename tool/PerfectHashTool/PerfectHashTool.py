#!env python3
# -*- coding: utf-8 -*-
"""
Modified from https://github.com/ilanschnell/perfect-hash/blob/master/perfect_hash.py
"""
import os
import sys
import json
import random
import string
import argparse
from collections import defaultdict


class Graph(object):
    """
    Implements a graph with 'N' vertices.  First, you connect the graph with
    edges, which have a desired value associated.  Then the vertex values
    are assigned, which will fail if the graph is cyclic.  The vertex values
    are assigned such that the two values corresponding to an edge add up to
    the desired edge value (mod N).
    """
    def __init__(self, N):
        self.N = N                     # number of vertices

        # maps a vertex number to the list of tuples (vertex, edge value)
        # to which it is connected by edges.
        self.adjacent = defaultdict(list)

    def connect(self, vertex1, vertex2, edge_value):
        """
        Connect 'vertex1' and 'vertex2' with an edge, with associated
        value 'value'
        """
        # Add vertices to each other's adjacent list
        self.adjacent[vertex1].append((vertex2, edge_value))
        self.adjacent[vertex2].append((vertex1, edge_value))

    def assign_vertex_values(self):
        """
        Try to assign the vertex values, such that, for each edge, you can
        add the values for the two vertices involved and get the desired
        value for that edge, i.e. the desired hash key.
        This will fail when the graph is cyclic.
        This is done by a Depth-First Search of the graph.  If the search
        finds a vertex that was visited before, there's a loop and False is
        returned immediately, i.e. the assignment is terminated.
        On success (when the graph is acyclic) True is returned.
        """
        self.vertex_values = self.N * [-1]  # -1 means unassigned

        visited = self.N * [False]

        # Loop over all vertices, taking unvisited ones as roots.
        for root in range(self.N):
            if visited[root]:
                continue

            # explore tree starting at 'root'
            self.vertex_values[root] = 0    # set arbitrarily to zero

            # Stack of vertices to visit, a list of tuples (parent, vertex)
            tovisit = [(None, root)]
            while tovisit:
                parent, vertex = tovisit.pop()
                visited[vertex] = True

                # Loop over adjacent vertices, but skip the vertex we arrived
                # here from the first time it is encountered.
                skip = True
                for neighbor, edge_value in self.adjacent[vertex]:
                    if skip and neighbor == parent:
                        skip = False
                        continue

                    if visited[neighbor]:
                        # We visited here before, so the graph is cyclic.
                        return False

                    tovisit.append((vertex, neighbor))

                    # Set new vertex's value to the desired edge value,
                    # minus the value of the vertex we came here from.
                    self.vertex_values[neighbor] = (edge_value - self.vertex_values[vertex]) % self.N

        # check if all vertices have a valid value
        for vertex in range(self.N):
            assert self.vertex_values[vertex] >= 0

        # We got though, so the graph is acyclic,
        # and all values are now assigned.
        return True


class StrSaltHash(object):
    """
    Random hash function generator.
    Simple byte level hashing: each byte is multiplied to another byte from
    a random string of characters, summed up, and finally modulo NG is
    taken.
    """
    chars = string.ascii_letters + string.digits

    def __init__(self, N):
        self.N = N
        self.salt = ''

    def __call__(self, key):
        while len(self.salt) < len(key):  # add more salt as necessary
            self.salt += random.choice(self.chars)
        return sum(ord(self.salt[i]) * ord(c) for i, c in enumerate(key)) % self.N


class TooManyIterationsError(Exception):
    pass


def generate_hash(keys, trials=5, verbose=False):
    """
    Return hash functions f1 and f2, and G for a perfect minimal hash.
    Input is an iterable of 'keys', whos indicies are the desired hash values.
    'Hash' is a random hash function generator, that means Hash(N) returns a
    random hash function which returns hash values from 0..N-1.
    """
    if not isinstance(keys, (list, tuple)):
        raise TypeError("list or tuple expected")
    NK = len(keys)
    if NK != len(set(keys)):
        raise ValueError("duplicate keys")
    for key in keys:
        if not isinstance(key, str):
            raise TypeError("key a not string: %r" % key)

    # the number of vertices in the graph G
    NG = NK + 1
    if verbose:
        sys.stderr.write('NG = %d\n' % NG)

    trial = 0  # Number of trial graphs so far
    while True:
        if (trial % trials) == 0:   # trials failures, increase NG slightly
            if trial > 0:
                NG = max(NG + 1, int(1.05 * NG))
            if verbose:
                sys.stderr.write('\nGenerating graphs NG = %d ' % NG)
        trial += 1

        if NG > 100 * (NK + 1):
            raise TooManyIterationsError("%d keys" % NK)

        if verbose:
            sys.stderr.write('.')
            sys.stderr.flush()

        G = Graph(NG)   # Create graph with NG vertices
        f1 = StrSaltHash(NG)   # Create 2 random hash functions
        f2 = StrSaltHash(NG)

        # Connect vertices given by the values of the two hash functions
        # for each key.  Associate the desired hash value with each edge.
        for hashval, key in enumerate(keys):
            G.connect(f1(key), f2(key), hashval)

        # Try to assign the vertex values.  This will fail when the graph
        # is cyclic.  But when the graph is acyclic it will succeed and we
        # break out, because we're done.
        if G.assign_vertex_values():
            break

    if verbose:
        sys.stderr.write('\nAcyclic graph found after %d trials.\n' % trial)
        sys.stderr.write('NG = %d\n' % NG)

    # Sanity check the result by actually verifying that all the keys
    # hash to the right value.
    for hashval, key in enumerate(keys):
        assert hashval == (G.vertex_values[f1(key)] + G.vertex_values[f2(key)]) % NG
    return f1, f2, G.vertex_values


def determin_int_type(data):
    max_d = 0
    for d in data:
        if d > max_d:
            max_d = d
    if max_d <= 0xFF:
        return 'uint8_t'
    elif max_d <= 0xFFFF:
        return 'uint16_t'
    else:
        assert(max_d <= 0xFFFFFFFF)
        return 'uint32_t'


def str_to_char_seq(s):
    ret = []
    for ch in s:
        ret.append(f"'{ch}'")
    return ', '.join(ret)


def generate_code(output_header_file, output_src_file, keys, enums, enum_name, hash_func_name):
    keys_only = list(keys.keys())

    # 生成完美哈希函数
    f1, f2, G = generate_hash(keys_only)

    assert f1.N == f2.N == len(G)
    try:
        salt_len = len(f1.salt)
        assert salt_len == len(f2.salt)
    except TypeError:
        salt_len = None

    # 写出头文件
    with open(output_header_file, 'w', encoding='utf-8') as f:
        f.write(f'#pragma once\n')
        f.write(f'#include <cstdint>\n')
        f.write(f'#include <optional>\n')
        f.write(f'#include <string_view>\n')
        f.write(f'\n')
        f.write(f'enum class {enum_name}\n')
        f.write(f'{{\n')
        for k in enums:
            v = enums[k]
            f.write(f'    {k} = {v},\n')
        f.write(f'}};\n')
        f.write(f'\n')
        f.write(f'extern std::optional<{enum_name}> {hash_func_name}(std::string_view key) noexcept;\n')
        f.write(f'\n')

    # 写出源文件
    with open(output_src_file, 'w', encoding='utf-8') as f:
        f.write(f'#include "{os.path.relpath(output_header_file, os.path.dirname(output_src_file))}"\n')
        f.write(f'\n')
        f.write(f'#include <cassert>\n')
        f.write(f'\n')
        f.write(f'static const char* kKeys[] = {{\n')
        for k in keys:
            f.write(f'    "{k}",\n')
        f.write(f'}};\n')
        f.write(f'static const {enum_name} kEnums[] = {{\n')
        for k in keys:
            f.write(f'    {enum_name}::{keys[k]},\n')
        f.write(f'}};\n')
        f.write(f'\n')
        f.write(f'static const {determin_int_type(G)} kPerfectHashG[] = {{\n')
        f.write(f'    ')
        for i in range(0, len(G)):
            f.write(f'{G[i]}, ')
            if (i + 1) % 10 == 0:
                f.write(f'\n    ')
        f.write(f'\n}};\n')
        f.write(f'\n')
        f.write(f'static const size_t kPerfectHashGCount = {len(G)};\n')
        f.write(f'static_assert(sizeof(kPerfectHashG) / sizeof(kPerfectHashG[0]) == kPerfectHashGCount);\n')
        f.write(f'\n')
        f.write(f'static const char kHashSalt1[] = {{ {str_to_char_seq(f1.salt)} }};\n')
        f.write(f'static const char kHashSalt2[] = {{ {str_to_char_seq(f2.salt)} }};\n')
        f.write(f'\n')
        f.write('template <size_t N>\n')
        f.write('static inline size_t HashF(std::string_view key, const char(&salt)[N]) noexcept\n')
        f.write('{\n')
        f.write('    uint64_t sum = 0;\n')
        f.write('    for (size_t i = 0, j = 0; i < key.length(); ++i)\n')
        f.write('    {\n')
        f.write('        auto newSum = sum + (static_cast<int>(salt[j++]) * static_cast<int>(key[i]));\n')
        f.write('        assert(newSum >= sum);\n')
        f.write('        sum = newSum;\n')
        f.write('        if (j >= N) j = 0;\n')
        f.write('    }\n')
        f.write('    return sum % kPerfectHashGCount;\n')
        f.write('}\n')
        f.write('\n')
        f.write(f'std::optional<{enum_name}> {hash_func_name}(std::string_view key) noexcept\n')
        f.write(f'{{\n')
        f.write(f'    auto index = (kPerfectHashG[HashF(key, kHashSalt1)] + kPerfectHashG[HashF(key, kHashSalt2)]) % kPerfectHashGCount;\n')
        f.write(f'    if (sizeof(kKeys) / sizeof(kKeys[0]) <= index)\n')
        f.write(f'        return {{}};\n')
        f.write(f'    if (kKeys[index] == key)\n')
        f.write(f'        return kEnums[index];\n')
        f.write(f'    return {{}};\n')
        f.write(f'}}\n')
        f.write('\n')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--define", required=True, type=str, help="Definition file")
    parser.add_argument("-i", "--header", required=True, type=str, help="Output header path")
    parser.add_argument("-s", "--source", required=True, type=str, help="Output source path")
    args = parser.parse_args()

    # 从文件加载配置
    with open(args.define, 'r', encoding='utf-8') as f:
        decl = json.load(f)

    # 生成
    generate_code(args.header, args.source, decl["keys"], decl["enums"], decl["enum_name"], decl["hasher_name"])


if __name__ == '__main__':
    main()
