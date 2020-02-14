export DNSCHAT_COUNTER=$(cat ~/.dnschat_counter)
export DNSCHAT_USER=$1

export DNSCHAT_MODE=$2

if [ -z $DNSCHAT_MODE ]
then
  export DNSCHAT_MODE=normal
fi

dnschat_send() {
  export DNSCHAT_COUNTER=$(($DNSCHAT_COUNTER + 1))
  case $DNSCHAT_MODE in
    safe) dig txt +noidnin +noidnout "$@".$DNSCHAT_USER.$DNSCHAT_COUNTER.dnschat.foo.gifter.cf ;;
    *) dig txt +noidnin +noidnout "$@".$DNSCHAT_USER.$DNSCHAT_COUNTER.dnschat @dns.c3murk.dev ;;
  esac
  echo $DNSCHAT_COUNTER > ~/.dnschat_counter
}

case $DNSCHAT_MODE in
  safe) export DNSCHAT_POLL="sh poll-safe.sh" ;;
  *) export DNSCHAT_POLL="sh poll.sh" ;;
esac

dnschat_prompt() {
  clear
  $DNSCHAT_POLL
  echo -n "dnschat> "
  read input
  if [ "$input" = ".exit" ]
  then
     exit 0;
  fi
  dnschat_send "$input"
}

trap dnschat_prompt INT

while(true)
do
  watch -t -n 1 $DNSCHAT_POLL
done
