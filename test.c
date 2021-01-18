#include <stdio.h>
#define NORMAL "\x1b[0m"
#define BOLD "\x1b[1m"
#define BLACK "\x1b[30m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define WHITE "\x1b[37m"

//NORMAL, BOLD, BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE

int main() {
	printf("%sA%sA%sA%sA%sA%sA%sA%sA%sA%sA\n", NORMAL, BOLD, BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE);
	return 0;
}