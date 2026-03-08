#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/reboot.h>
#include <sys/syscall.h>
#include <susfs_defs.h>
#include <susfs_utils.h>
#include "open_redirect.h"

#define CMD_SUSFS_ADD_OPEN_REDIRECT 0x555c0

struct st_susfs_open_redirect {
	unsigned long           target_ino;
	char                    target_pathname[SUSFS_MAX_LEN_PATHNAME];
	char                    redirected_pathname[SUSFS_MAX_LEN_PATHNAME];
	int                     err;
};

void open_redirect_print_help(void){
	log("    add_open_redirect </target/path> </redirected/path>\n");
	log("      |--> Redirect the target path to be opened with user defined path\n");
	log("      * Important Notes *\n");
	log("      - Only effective for current process with uid <= 2000\n");
	log("\n");
}

static void print_help(void){
	print_help_banner();
	open_redirect_print_help();
}

int add_open_redirect(int argc, char *argv[]) {
	struct st_susfs_open_redirect info = {0};
	struct stat sb;
	char target_pathname[PATH_MAX], *p_abs_target_pathname;
	char redirected_pathname[PATH_MAX], *p_abs_redirected_pathname;

	if (argc != 4) {
		print_help();
		return -EINVAL;
	}

	p_abs_target_pathname = realpath(argv[2], target_pathname);
	if (p_abs_target_pathname == NULL) {
		perror("realpath");
		return errno;
	}
	strncpy(info.target_pathname, target_pathname, SUSFS_MAX_LEN_PATHNAME-1);
	p_abs_redirected_pathname = realpath(argv[3], redirected_pathname);
	if (p_abs_redirected_pathname == NULL) {
		perror("realpath");
		return errno;
	}
	strncpy(info.redirected_pathname, redirected_pathname, SUSFS_MAX_LEN_PATHNAME-1);
	info.err = get_file_stat(info.target_pathname, &sb);
	if (info.err) {
		log("[-] failed to get stat from path: '%s'\n", info.target_pathname);
		return info.err;
	}
	info.target_ino = sb.st_ino;
	info.err = ERR_CMD_NOT_SUPPORTED;
	syscall(SYS_reboot, KSU_INSTALL_MAGIC1, SUSFS_MAGIC, CMD_SUSFS_ADD_OPEN_REDIRECT, &info);
	PRT_MSG_IF_CMD_NOT_SUPPORTED(info.err, CMD_SUSFS_ADD_OPEN_REDIRECT);
	return info.err;
}

