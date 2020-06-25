while true; do
  ssh -t -i ../../lckey/repo ec2-user@leu-235.com "sudo lsof -n -i :1847 | grep LISTEN | awk '{ print \$2 }' | sort | uniq | xargs kill"
  ssh -o "ExitOnForwardFailure yes" -i ../../lckey/repo -N -R :1847:localhost:1847 ec2-user@leu-235.com
done
