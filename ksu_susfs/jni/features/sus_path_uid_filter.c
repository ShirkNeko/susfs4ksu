#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/reboot.h>
#include <sys/syscall.h>
#include <errno.h>
#include <susfs_defs.h>
#include <susfs_utils.h>
#include "sus_path_uid_filter.h"

#define CMD_SUSFS_SET_SUS_PATH_UID_FILTER 0x55554

/* The maximum size of the app package names buffer, which is the maximum value of a u16 */
#define APP_STR_BUF_SIZE 65535

struct st_susfs_sus_path_uid_filter {
	char                    app_strings[APP_STR_BUF_SIZE];
	int                     err;
};

void sus_path_uid_filter_print_help(void){
	log("    set_sus_path_uid_filter <app_package_names_separated_by_comma>\n");
	log("      |--> Only uids of added package name will be visible to sus paths, and vice versa\n");
	log("      * Important Notes *\n");
	log("      - App uid is resolved from /data/system/packages.list\n");
	log("      - Only effective for all processes with uid >= 10000\n");
	log("\n");
}

static void print_help(void){
	print_help_banner();
	sus_path_uid_filter_print_help();
}

int set_sus_path_uid_filter(int argc, char *argv[]) {
	struct st_susfs_sus_path_uid_filter info = {0};

	if (argc != 3) {
		print_help();
		return -EINVAL;
	}

	if (*argv[2] == '\0') {
		log("[-] argv[2] is empty'\n");
		return -EINVAL;
	}

	if (strlen(argv[2]) >= APP_STR_BUF_SIZE) {
		log("[-] the app package names are too long, the maximum length is %d\n", APP_STR_BUF_SIZE - 1);
		return -EINVAL;
	}

	strncpy(info.app_strings, argv[2], APP_STR_BUF_SIZE-1);
	info.err = ERR_CMD_NOT_SUPPORTED;
	syscall(SYS_reboot, KSU_INSTALL_MAGIC1, SUSFS_MAGIC, CMD_SUSFS_SET_SUS_PATH_UID_FILTER, &info);
	PRT_MSG_IF_CMD_NOT_SUPPORTED(info.err, CMD_SUSFS_SET_SUS_PATH_UID_FILTER);
	return info.err;
}
