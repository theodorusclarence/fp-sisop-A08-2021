#!/bin/bash
# awk -F";" '
# BEGIN {}
# { 
#   if (NF == 1) {
    
#   }
#   if (NR == 2) next 
#   # print $0;
#   print $1";"$2";"$3;
# }
# END {
    
# }' database/databases/haha/testing



awk -F";" '
NR==1 {
    for (i=1; i<=NF; i++) {
        f[$i] = i
    }
}
{ print $(f["a"]), $(f["c"]) }' database/databases/haha/testing