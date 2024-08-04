set -ex
cargo fmt
scp src/config.rs ec2-user@leu-235.com:./OC1/src/
ssh -t ec2-user@leu-235.com "tmux send-keys -t OC1 C-c C-m"
ssh -t ec2-user@leu-235.com "tmux send-keys -t OC1 cargo\ run C-m"
