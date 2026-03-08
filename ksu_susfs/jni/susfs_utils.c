#include <errno.h>
#include "susfs_defs.h"
#include "susfs_utils.h"

int isNumeric(char* str) {
        // Check if the string is empty
        if (str[0] == '\0') {
                return 0;
        }

        // Check each character in the string
        for (int i = 0; str[i] != '\0'; i++) {
                // If any character is not a digit, return false
                if (!isdigit(str[i])) {
                        return 0;
                }
        }

        // All characters are digits, return true
        return 1;
}

int get_file_stat(char *pathname, struct stat* sb) {
        if (stat(pathname, sb) != 0) {
                return errno;
        }
        return 0;
}

void print_help_banner(void){
	log("usage: %s <CMD> [CMD options]\n", TAG);
	log("  <CMD>:\n");
}
