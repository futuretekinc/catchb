#!/bin/sh

cat $1 | awk '{print $0\n}' | xargs echo 
