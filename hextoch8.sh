#!/usr/bin/env bash

ch8name=$(echo $1 | cut -d'.' -f1)
xxd -r -ps $1 "${ch8name}.ch8"

