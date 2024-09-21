#!/bin/bash

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 baseline_result benchmark_result"
  exit 1
fi

BASE_LABELS=($(grep "Result" "$1" | awk '{for(i=1;i<NF;i++) if($i=="Result") print $(i-1)}'))
BASE_RES=($(grep "Average" "$1" | awk '{print $2}'))
RES_LABELS=($(grep "Result" "$2" | awk '{for(i=1;i<NF;i++) if($i=="Result") print $(i-1)}'))
RES=($(grep "Average" "$2" | awk '{print $2}'))

norm_res=()

for i in "${!BASE_LABELS[@]}"; do
  base_label="${BASE_LABELS[$i]}"

  for j in "${!RES_LABELS[@]}"; do
    res_label="${RES_LABELS[$j]}"

    if [[ "$base_label" == "$res_label" ]]; then
      y1="${BASE_RES[$i]}"
      y2="${RES[$j]}"

      #echo "$base_label - y1: $y1, y2: $y2"

      if [[ "$base_label" == "Kernbench" || "$base_label" == "Hackbench" ]]; then
        norm=$(echo "scale=4; $y2 / $y1" | bc)
      else
        norm=$(echo "scale=4; $y1 / $y2" | bc)
      fi

      norm_res+=("$base_label : $norm")
      break
    fi
  done
done

echo "" >> $2
echo "Normalized Result (against $(basename $1)):" | tee -a $2
printf '%s\n' "${norm_res[@]}" | tee -a $2
