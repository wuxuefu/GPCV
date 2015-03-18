#!/bin/sh

DIR="$1"

if [ -d "$DIR" ]
then
	echo 1
else
	echo 0
fi
