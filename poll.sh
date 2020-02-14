dig txt +noquestion poll.dnschat.foo.gifter.cf @dns.c3murk.dev | \
grep -v '^;.*' | \
awk -vFPAT='([^"][^\t]+)|(\"[^\"]+\")"' '{print $1,$5}'
