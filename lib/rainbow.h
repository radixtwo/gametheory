
#define ANSI_CLEAR()        printf("\033[H\033[J")
#define ANSI_CURSOR(x,y)    printf("\033[%d;%dH",(x),(y))

#define ANSI_ERASE_DISPLAY "\033[2J"
#define ANSI_COLOR_BOLD    "\033[1m"
#define ANSI_COLOR_RED     "\033[31m"
#define ANSI_COLOR_GREEN   "\033[32m"
#define ANSI_COLOR_YELLOW  "\033[33m"
#define ANSI_COLOR_BLUE    "\033[34m"
#define ANSI_COLOR_MAGENTA "\033[35m"
#define ANSI_COLOR_CYAN    "\033[36m"
#define ANSI_COLOR_RESET   "\033[0m"

