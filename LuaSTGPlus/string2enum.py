import sys
import json
import math
import random

def maxlen(str_list):
    ret = 0
    for s in str_list:
        if len(s) > ret:
            ret = len(s)
    return ret

def combination(seq, length):
    if not length:
        yield []
    else:
        for i in xrange(len(seq)):
            for result in combination(seq[i+1:], length-1):
                yield [seq[i]] + result

def charat(key, idx):
    if idx >= len(key):
        return u'\0'
    else:
        return key[idx]

def is_prime(n):
    if n <= 1:
        return False
    for i in range(2, int(math.sqrt(n)) + 1):
        if n % i == 0:
            return False
    return True

class union_set:
    def __init__(self, count):
        self._obj = [x for x in range(0, count)]

    def find(self, x):
        if x != self._obj[x]:
            self._obj[x] = self.find(self._obj[x])
            return self._obj[x]
        else:
            return x

    def connect(self, a, b):
        x = self.find(a)
        y = self.find(b)
        self._obj[x] = y

    def is_connected(self, a, b):
        return self.find(a) == self.find(b)

# find best indices
# !!NEED TO BE IMPROVED!!
def find_best_indices(key_list):
    n = maxlen(key_list)
    seq = [x for x in range(0, n)]
    for cnt in range(1, n + 1):
        for comb in combination(seq, cnt):
            test_set = set()
            fail = False
            for key in key_list:
                comb_str = ""
                for idx in comb:
                    comb_str += charat(key, idx)
                if comb_str in test_set:
                    fail = True
                    break
                test_set.add(comb_str)
            if not fail:
                print("*** Best indices found: " + str(comb))
                return comb
    return None

def keyhash(key, rand_table, idx_list, n):
    ret = 0
    for i in range(0, len(idx_list)):
        ret += ord(charat(key, idx_list[i])) * rand_table[i]
    return ret % n

def generate_graph(key_list, idx_list, factor):
    n = int(len(key_list) * factor + 1)
    failed = True
    while not is_prime(n):
        n += 1
    print("*** 'n' selected: " + str(n))
    print("*** Start iterating...")
    iter_cnt = 0
    while failed:
        iter_cnt += 1
        print("trying iterating, step %d..." % iter_cnt)
        # generate random table
        T1 = [random.randint(1, 255) for i in range(0, len(idx_list))]
        T2 = [random.randint(1, 255) for i in range(0, len(idx_list))]
        # generate empty graph
        adj_matrix = [list() for i in range(0, n)]
        uniset = union_set(n)
        # calcu each key
        index = 0
        failed = False
        for key in key_list:
            hash1 = keyhash(key, T1, idx_list, n)
            hash2 = keyhash(key, T2, idx_list, n)
            # connect hash1 and hash2
            if uniset.is_connected(hash1, hash2):
                failed = True
                break
            uniset.connect(hash1, hash2)
            # make edge
            edge = { "hash1" : hash1, "hash2" : hash2, "key" : key, "value" : index }
            adj_matrix[hash1].append(edge)
            adj_matrix[hash2].append(edge)
            index += 1
    print("*** Graph generated")
    return T1, T2, adj_matrix

def find_func_g(adj_matrix, m):
    g = [0 for i in range(0, len(adj_matrix))]
    visited = [False for i in range(0, len(adj_matrix))]
    def visit_graph(v):
        visited[v] = True
        for adj in adj_matrix[v]:
            if v == adj["hash1"] and not visited[adj["hash2"]]:
                g[adj["hash2"]] = (adj["value"] - g[adj["hash1"]]) % m
                visit_graph(adj["hash2"])
            elif v == adj["hash2"] and not visited[adj["hash1"]]:
                g[adj["hash1"]] = (adj["value"] - g[adj["hash2"]]) % m
                visit_graph(adj["hash1"])

    print("*** Finding function 'g'...")
    for vert in range(0, len(adj_matrix)):
        if len(adj_matrix[vert]) != 0 and not visited[vert]:
            g[vert] = 0
            visit_graph(vert)
    print("*** Function 'g' generated")
    return g

def final_hash_func(key, idx_list, T1, T2, g_table, m):
    n = len(g_table)
    return (g_table[keyhash(key, T1, idx_list, n)] + g_table[keyhash(key, T2, idx_list, n)]) % m

def generated_mpf(filename):
    with open(filename, "r") as f:
        options = json.load(f)
    keys = []
    enums = []
    keydict = {}  # key->enum
    enumdict = {}  # enum->key
    for item in options["keys"]:
        keys.append(item[0])
        enums.append(item[1])
        keydict[item[0]] = item[1]
        enumdict[item[1]] = item[0]
    # step1: find best indices
    best_indices = find_best_indices(keys)
    # step2: generate random table and graph
    hash_table1, hash_table2, graph = generate_graph(keys, best_indices, options["factor"] if options.has_key("factor") else 2.0)
    # step3: generate function g
    hash_func_g = find_func_g(graph, len(keys))
    # check step
    for i in range(0, len(keys)):
        hash_check = final_hash_func(keys[i], best_indices, hash_table1, hash_table2, hash_func_g, len(keys))
        # print("key %s hash %d" % (keys[i], hash_check))
        assert(i == hash_check)
    # print results
    print("*** Results:")
    print("n = " + str(len(graph)))
    print("m = " + str(len(keys)))
    print("best_indices = " + str(best_indices))
    print("hash_table1 = " + str(hash_table1))
    print("hash_table2 = " + str(hash_table2))
    print("hashfunc_g = " + str(hash_func_g))
    # generate C++ source file
    print("*** generating C++ source file...")
    char_type = "wchar_t" if options["wide_char"] else "char"
    with open(options["output"], "w") as out_file:
        out_file.write(u"#pragma once\n")
        out_file.write(u"#include <cstring>\n")
        #out_file.write(u"#include <cstdint>\n")
        out_file.write(u"\n")
        out_file.write(u"namespace %s\n" % options["namespace"])
        out_file.write(u"{\n")
        out_file.write(u"\tenum class %s\n" % options["enum_name"])
        out_file.write(u"\t{\n")
        for i in range(0, len(enums)):
            out_file.write(u"\t\t%s = %d,\n" % (enums[i], i))
        out_file.write(u"\t\t_KEY_NOT_FOUND = -1\n")
        out_file.write(u"\t};\n")
        out_file.write(u"\n")
        out_file.write(u"\tinline %s %s(const %s* key)\n" % (options["enum_name"], options["hashfunc_name"], char_type))
        out_file.write(u"\t{\n")
        out_file.write(u"\t\tstatic const %s* s_orgKeyList[] =\n" % char_type)
        out_file.write(u"\t\t{\n")
        for i in range(0, len(keys)):
            out_file.write(u'\t\t\t%s"%s",\n' % (u'L' if options["wide_char"] else u'', keys[i]))
        out_file.write(u"\t\t};\n")
        out_file.write(u"\t\t\n")
        out_file.write(u"\t\tstatic const unsigned int s_bestIndices[] =\n")
        out_file.write(u"\t\t{")
        for i in range(0, len(best_indices)):
            if i % 10 == 0:
                out_file.write(u'\n')
                out_file.write(u'\t\t\t')
            out_file.write(u'%d, ' % (best_indices[i]))
        out_file.write(u"\n\t\t};\n")
        out_file.write(u"\t\t\n")
        out_file.write(u"\t\tstatic const unsigned int s_hashTable1[] =\n")
        out_file.write(u"\t\t{")
        for i in range(0, len(best_indices)):
            if i % 10 == 0:
                out_file.write(u'\n')
                out_file.write(u'\t\t\t')
            out_file.write(u'%d, ' % (hash_table1[i]))
        out_file.write(u"\n\t\t};\n")
        out_file.write(u"\t\t\n")
        out_file.write(u"\t\tstatic const unsigned int s_hashTable2[] =\n")
        out_file.write(u"\t\t{")
        for i in range(0, len(best_indices)):
            if i % 10 == 0:
                out_file.write(u'\n')
                out_file.write(u'\t\t\t')
            out_file.write(u'%d, ' % (hash_table2[i]))
        out_file.write(u"\n\t\t};\n")
        out_file.write(u"\t\t\n")
        out_file.write(u"\t\tstatic const unsigned int s_hashTableG[] =\n")
        out_file.write(u"\t\t{")
        for i in range(0, len(graph)):
            if i % 10 == 0:
                out_file.write(u'\n')
                out_file.write(u'\t\t\t')
            out_file.write(u'%d, ' % (hash_func_g[i]))
        out_file.write(u"\n\t\t};\n")
        out_file.write(u"\t\t\n")
        out_file.write(u"\t\tunsigned int f1 = 0, f2 = 0, len = %s(key);\n" % options["strlen"])
        out_file.write(u"\t\tfor (unsigned int i = 0; i < %d; ++i)\n" % len(best_indices))
        out_file.write(u"\t\t{\n")
        out_file.write(u"\t\t\tunsigned int idx = s_bestIndices[i];\n")
        out_file.write(u"\t\t\tif (idx < len)\n")
        out_file.write(u"\t\t\t{\n")
        out_file.write(u"\t\t\t\tf1 = (f1 + s_hashTable1[i] * (unsigned int)key[idx]) %% %d;\n"  % len(graph))
        out_file.write(u"\t\t\t\tf2 = (f2 + s_hashTable2[i] * (unsigned int)key[idx]) %% %d;\n"  % len(graph))
        out_file.write(u"\t\t\t}\n")
        out_file.write(u"\t\t\telse\n")
        out_file.write(u"\t\t\t\tbreak;\n")
        out_file.write(u"\t\t}\n")
        out_file.write(u"\t\t\n")
        out_file.write(u"\t\tunsigned int hash = (s_hashTableG[f1] + s_hashTableG[f2]) %% %d;\n" % len(keys))
        out_file.write(u"\t\tif (%s(s_orgKeyList[hash], key) == 0)\n" % options["strcmp"])
        out_file.write(u"\t\t\treturn static_cast<%s>(hash);\n" % options["enum_name"])
        out_file.write(u"\t\treturn %s::_KEY_NOT_FOUND;\n" % options["enum_name"])
        out_file.write(u"\t}\n")
        out_file.write(u"}\n")
    print("*** finished")

if len(sys.argv) != 2:
    print("Invalid command argument.")
    exit(-1)

generated_mpf(sys.argv[1])
