#!/bin/bash
set -xe
printf "Uninstalling...\n"
rm -f /usr/bin/uwufetch
rm -rf /usr/lib/uwufetch
rm -f /usr/lib/libfetch.so
rm -f /usr/include/fetch.h
rm -rf /etc/uwufetch
rm -f /usr/share/man/man1/uwufetch.1.gz
printf "Done!\n"

