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
#include "spoof_uname.h"

#define CMD_SUSFS_SET_UNAME 0x55590

#ifndef __NEW_UTS_LEN
#define __NEW_UTS_LEN 64
#endif

struct st_susfs_uname {
	char                    release[__NEW_UTS_LEN+1];
	char                    version[__NEW_UTS_LEN+1];
	int                     err;
};

void set_uname_print_help(void){
	log("    set_uname <release> <version>\n");
	log("      |--> Spoof uname for all processes, set string to 'default' to imply the function to use original string\n");
	log("      |--> NOTE: only 'release' and <version> are spoofed as others are no longer needed\n");
	log("      |--> e.g., set_uname '4.9.337-g3291538446b7' '#1 SMP PREEMPT Mon Oct 6 16:50:48 UTC 2025'\n");
	log("\n");
}

static void print_help(void){
	print_help_banner();
	set_uname_print_help();
}

int set_uname(int argc, char *argv[]) {
	struct st_susfs_uname info = {0};

	if (argc != 4) {
		print_help();
		return -EINVAL;
	}

	if (*argv[2] == '\0') {
		log("[-] argv[2] is empty'\n");
		return -EINVAL;
	}

	if (*argv[3] == '\0') {
		log("[-] argv[3] is empty'\n");
		return -EINVAL;
	}

	strncpy(info.release, argv[2], __NEW_UTS_LEN);
	strncpy(info.version, argv[3], __NEW_UTS_LEN);
	info.err = ERR_CMD_NOT_SUPPORTED;
	syscall(SYS_reboot, KSU_INSTALL_MAGIC1, SUSFS_MAGIC, CMD_SUSFS_SET_UNAME, &info);
	PRT_MSG_IF_CMD_NOT_SUPPORTED(info.err, CMD_SUSFS_SET_UNAME);
	return info.err;
}

