# 20 january 2020
# note: python 3

import fileinput
import re

r = re.compile('^Test\(([A-Za-z0-9_]+)\)$')
for line in fileinput.input():
	match = r.match(line)
	if match is not None:
		print('Test' + match.group(1))
