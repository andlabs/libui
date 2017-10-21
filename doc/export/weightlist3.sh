# 21 october 2017
sort -t$'\t' -k1,1 -k2,2 "$@" |
	column -t -s$'\t'
