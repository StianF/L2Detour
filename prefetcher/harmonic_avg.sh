awk 'BEGIN { n = 0; denom = 0 }; \
     { if (n == '$2') denom += 1 / $3; \
       n++; \
       if (n > '$1') n = 0; \
     } \
     END { print 12 / denom }'