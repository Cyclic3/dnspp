export DNSCHAT_COUNTER=$(cat ~/.dnschat_counter)

dnschat_send() {
  export DNSCHAT_COUNTER=$(($DNSCHAT_COUNTER + 1))
  dig txt +noidnin +noidnout "$@".harlan.$DNSCHAT_COUNTER.dnschat @dns.c3murk.dev
  echo $DNSCHAT_COUNTER > ~/.dnschat_counter
}

dnschat_prompt() {
  echo -n "dnschat> "
  read input
  if [ "$input" = "/exit" ]
  then
     exit 0;
  fi
  dnschat_send "$input"
}

trap dnschat_prompt INT

while(true)
do
  watch -n 2 "sh poll.sh"
done
