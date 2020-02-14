export DNSCHAT_COUNTER=$(cat ~/.dnschat_counter)
export DNSCHAT_USER=$1

dnschat_send() {
  export DNSCHAT_COUNTER=$(($DNSCHAT_COUNTER + 1))
  dig txt +noidnin +noidnout "$@".$DNSCHAT_USER.$DNSCHAT_COUNTER.dnschat @dns.c3murk.dev
  echo $DNSCHAT_COUNTER > ~/.dnschat_counter
}

dnschat_prompt() {
  clear
  sh poll.sh
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
  watch -t -n 1 "sh poll.sh"
done
