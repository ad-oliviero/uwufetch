sed 's/{NORMAL}/\x1b[0m/g;
s/{BOLD}/\x1b[1m/g;
s/{BLACK}/\x1b[30m/g;
s/{RED}/\x1b[31m/g;
s/{GREEN}/\x1b[32m/g;
s/{SPRING_GREEN}/\x1b[38;5;120m/g;
s/{YELLOW}/\x1b[33m/g;
s/{BLUE}/\x1b[34m/g;
s/{MAGENTA}/\x1b[0;35m/g;
s/{CYAN}/\x1b[36m/g;
s/{WHITE}/\x1b[37m/g;
s/{PINK}/\x1b[38;5;201m/g;
s/{LPINK}/\x1b[38;5;213m/g;
s/{\(BLOCK_VERTICAL\|BLOCK\)}/▇/g;
s/{BACKGROUND_GREEN}/\e[0;42m/g;
s/{BACKGROUND_RED}/\e[0;41m/g;
s/{BACKGROUND_WHITE}/\e[0;47m/g'
