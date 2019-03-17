# 21 october 2017
gawk '
BEGIN { FS = "\t+" }
!/float..as/ { next }
{ i = 0; if ($1 == "") i++ }
(NF-i) != 2 { next }
{ print }
' "$@"
