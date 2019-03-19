#!/bin/sh
now=$(date '+%Y%m%d %H:%M')

cat >builddate.c <<EOF
const char * build_date="$now";
EOF

