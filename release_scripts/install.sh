#!/bin/bash
set -xe
printf "Installing binary...\n"
install -CDvm 755 uwufetch -t /usr/bin
install -CDvm 755 libfetch.a -t /usr/lib
install -CDvm 755 libfetch.so -t /usr/lib
install -CDvm 644 $(find res/ -type f -print) -t /usr/lib/uwufetch
install -CDvm 644 $(find res/ascii -type f -print) -t /usr/lib/uwufetch/ascii
install -CDvm 644 default.config -t /etc/uwufetch/config
install -CDvm 644 uwufetch.1.gz -t /usr/share/man/man1
install -CDvm 644 fetch.h logging.h -t /usr/include/libfetch
printf "Done!\n"

