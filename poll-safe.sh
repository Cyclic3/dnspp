dig txt +noquestion poll-concat.dnschat.dns.c3murk.dev | \
grep -v '^;.*' | \
sed 's/.*TXT\t//g' | \
sed -E 's/ *"(([^"]|\\")*)+"/\1\n/g' | \
sed '/^$/d'
