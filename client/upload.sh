scp -i ../../lckey/cyb1.pem client.lua ec2-user@cyb1.net:/home/ec2-user/client.lua
ssh -t -i ../../lckey/cyb1.pem ec2-user@cyb1.net "sudo mv /home/ec2-user/client.lua /usr/share/nginx/html/oc-scripts/client.lua"
