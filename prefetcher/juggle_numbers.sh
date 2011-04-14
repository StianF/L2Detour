cat N\ for\ speedup | \
  awk 'BEGIN { n = 0; last = 0; print "S,\tN\tSpeedup\tDiff" }; \
  { if (n == 0) denom = $3; \
  print $1 "\t" $2 "\t" $3 / denom "\t" $3 / denom - last; \
  last = $3 / denom; \
  n++; \
  if (n > '$1') { n = 0; last = 0 } }'