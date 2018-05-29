#!/bin/bash

echo "Checking..."
latest=`curl "https://api.github.com/repos/masonicboom/ipscrub/releases/latest" 2>/dev/null | grep --extended-regexp -o "\"tag_name\":\s+\".*\"" | cut -d '"' -f 4`
current=`git tag | sort | tail -n 1`

if [ $current != $latest ]; then
    echo "Current version is $current; latest is $latest. Check https://github.com/masonicboom/ipscrub/releases to see if you should update."
    exit 1
else
    echo "You are on the latest version of ipscrub ($current)."
fi