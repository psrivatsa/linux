#!/bin/bash
\rm -rf sidx_trace
mkdir sidx_trace

while IFS='' read -r line || [[ -n "$line" ]]; do
    hw_sidx="$(echo $line | cut -d'|' -f3)" 
    #echo "Text read from file: $line $hw_idx"
    if ! [[ $hw_sidx =~ 'hw_sidx' ]] ; then
      echo $line >> sidx_trace/${hw_sidx}_trace.csv
    fi
done < "$1"
