#! /usr/bin/python
# -*- coding: utf-8 -*-
# ©2022-2023 Yuichiro Nakada

# python3 make_single.py .

import re
import os
import sys
import glob

path = sys.argv[1]
files = glob.glob(path + '/*.[ch]')
#print(files)

# check dependencies
pattern = '(?<=include ").*(?=")'
d = {}
for p in files:
    with open(p) as f:
        list = []
        for i, line in enumerate(f):
            if '#include "' in line:
                #break
                #print(line)
                result = re.search(pattern, line.strip()).group()
                if result in p: continue
                list.append(result)
        d[os.path.basename(p)] = list

print('Dependencies: ')
print(d)

# check orders
order = []
for k, v in d.items():
    if len(v) == 0: # dependencies is 0
        if k not in order: order.append(k)
        continue

for k2, v2 in d.items():
    for k, v in d.items():
        ordered = 0
        for l in v:
            if l in order: ordered = ordered + 1
        if len(v) == ordered:
            if k not in order: order.append(k)

print('')
print('Orders: ')
print(order)

with open('/tmp/merged.h', 'w') as f_new:
    #for f in files:
    for f in order:
        f_new.write('// '+os.path.basename(f)+'\n')
        with open(f, 'r') as f_org:
            lines = f_org.readlines()
            for id, line in enumerate(lines):
                if '#include "' in line: lines[id] = '// '+line
            f_new.writelines(lines)
        f_new.write('\n\n')
