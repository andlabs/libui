# 7 april 2019

BEGIN {
	RS = ""
	FS = "\n +- "
}

/^- job:/ {
	for (i = 1; i <= NF; i++) {
		if (!(i in nextindex)) {
			# fast path for first occurrence
			lines[i, 0] = $i
			nextindex[i] = 1
			if (maxIndex < i)
				maxIndex = i
			continue
		}
		found = 0
		for (j = 0; j < nextindex[i]; j++)
			if (lines[i, j] == $i) {
				found = 1
				break
			}
		if (!found) {
			lines[i, nextindex[i]] = $i
			nextindex[i]++
		}
	}
}

END {
	for (i = 1; i <= maxIndex; i++) {
		if (nextindex[i] == 1) {
			# only one entry here, just print it
			print "- " lines[i, 0]
			continue
		}
		print "{"
		for (j = 0; j < nextindex[i]; j++)
			print "- " lines[i, j]
		print "}"
	}
}
