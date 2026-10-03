#!/bin/bash
# cap2.sh — usage: ./cap2.sh main

bundle exec cap "$1" deploy
if [ $? -ne 0 ]; then
  echo "cap $1 deploy failed — skipping the -standby deploy."
  read -p "Press Enter to close..."
  exit 1
fi

bundle exec cap "$1-standby" deploy
if [ $? -ne 0 ]; then
  echo "cap $1-standby deploy failed."
  read -p "Press Enter to close..."
  exit 1
fi
