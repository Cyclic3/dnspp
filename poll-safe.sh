dig txt +noquestion poll-concat.dnschat.foo.gifter.cf | \
grep -v '^;.*' | \
sed 's/.*TXT\t//g' | \
sed -E 's/ *"(([^"]|\\")*)+"/\1\n/g' | \
sed '/^$/d'
