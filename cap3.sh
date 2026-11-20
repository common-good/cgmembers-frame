#!/bin/bash
# cap3.sh - usage: ./cap3.sh   (deploys main, then main-standby, then demo, so demo always runs what main runs)

for s in main main-standby demo; do
  bundle exec cap "$s" deploy
  if [ $? -ne 0 ]; then
    echo "cap $s deploy failed - stopping."
    read -p "Press Enter to close..."
    exit 1
  fi
done
