# 21 october 2017
gawk '
{
	gsub(/float..as\(/, "")
	gsub(/,/, "", $(NF - 1))
	gsub(/\)$/, "")
	split($0, parts, /:/)
	print $(NF - 1) "\t" $NF "\t" parts[1]
}
' "$@"
