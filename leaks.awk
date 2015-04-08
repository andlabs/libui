# 7 april 2015

$2 == "alloc" {
	if ($1 in A) {
		problem($1 " already allocated (" A[$1] "); allocated at " NR)
		next
	}
	A[$1] = type()
	next
}

$2 == "realloc" {
	if (!($1 in A)) {
		problem($1 " not yet allocated; reallocated at " NR)
		next
	}
	if ($3 in A) {
		problem($3 " already allocated (" A[$3] "); reallocated at " NR)
		next
	}
	t = A[$1]
	delete A[$1]
	A[$3] = t
	next
}

$2 == "free" {
	if (!($1 in A)) {
		problem($1 " not yet allocated; freed at " NR)
		next
	}
	delete A[$1]
	next
}

{ problem("unrecognized line " $0 " at " NR) }

END {
	for (i in A)
		problem("leaked " A[i] " at " i)
	close("/dev/stderr")
	if (hasProblems)
		exit 1
}

function problem(s) {
	print s > "/dev/stderr"
	hasProblems = 1
}

function type(		s, i) {
	s = $3
	for (i = 4; i <= NF; i++)
		s = s " " $i
	return s
}
