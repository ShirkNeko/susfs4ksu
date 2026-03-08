#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/reboot.h>
#include <sys/syscall.h>
#include <errno.h>
#include <susfs_defs.h>
#include <susfs_utils.h>
#include "enable_log.h"

#define CMD_SUSFS_ENABLE_LOG 0x555a0

struct st_susfs_log {
	bool                    enabled;
	int                     err;
};

void enable_log_print_help(void){
	log("    enable_log <0|1>\n");
	log("      |--> 0: disable susfs log in kernel\n");
	log("      |--> 1: enable susfs log in kernel\n");
	log("\n");
}

static void print_help(void){
	print_help_banner();
	enable_log_print_help();
}

int enable_log(int argc, char *argv[]) {
	struct st_susfs_log info = {0};

	if (argc != 3) {
		print_help();
		return -EINVAL;
	}

	if (strcmp(argv[2], "0") && strcmp(argv[2], "1")) {
		print_help();
		return -EINVAL;
	}
	info.enabled = atoi(argv[2]);
	info.err = ERR_CMD_NOT_SUPPORTED;
	syscall(SYS_reboot, KSU_INSTALL_MAGIC1, SUSFS_MAGIC, CMD_SUSFS_ENABLE_LOG, &info);
	PRT_MSG_IF_CMD_NOT_SUPPORTED(info.err, CMD_SUSFS_ENABLE_LOG);
	return info.err;
}

