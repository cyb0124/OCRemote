scp -i ../../lckey/repo client.lua ec2-user@leu-235.com:/home/ec2-user/client.lua
ssh -t -i ../../lckey/repo ec2-user@leu-235.com "sudo mv /home/ec2-user/client.lua /usr/share/nginx/leu-235/oc-scripts/client.lua"
