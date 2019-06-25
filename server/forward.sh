while true; do
  ssh -t -i ../../lckey/cyb1.pem ec2-user@cyb1.net "sudo lsof -n -i :1847 | grep LISTEN | awk '{ print \$2 }' | sort | uniq | xargs kill"
  ssh -o "ExitOnForwardFailure yes" -i ../../lckey/cyb1.pem -N -R :1847:localhost:1847 ec2-user@cyb1.net
done
