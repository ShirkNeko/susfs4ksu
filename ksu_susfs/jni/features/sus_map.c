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
#include "sus_map.h"

#define CMD_SUSFS_ADD_SUS_MAP 0x60020

struct st_susfs_sus_map {
	char                    target_pathname[SUSFS_MAX_LEN_PATHNAME];
	int                     err;
};

void sus_map_print_help(void){
	log("    add_sus_map </path/to/actual/library>\n");
	log("      |--> added real file path which gets mmapped will be hidden from /proc/self/[maps|smaps|smaps_rollup|map_files|mem|pagemap]\n");
	log("      |--> e.g., add_sus_map '/data/adb/modules/my_module/zygisk/arm64-v8a.so'\n");
	log("      * Important Notes *\n");
	log("      - It does NOT support hiding for anon memory.\n");
	log("      - It does NOT hide any inline hooks or plt hooks cause by the injected library itself\n");
	log("      - It may not be able to evade detections by apps that implement a good injection detection\n");
	log("      - Only effective for umounted process with uid >= 10000\n");
	log("\n");
}

static void print_help(void){
	print_help_banner();
	sus_map_print_help();
}

int add_sus_map(int argc, char *argv[]) {
	struct st_susfs_sus_map info = {0};

	if (argc != 3) {
		print_help();
		return -EINVAL;
	}

	strncpy(info.target_pathname, argv[2], SUSFS_MAX_LEN_PATHNAME-1);
	info.err = ERR_CMD_NOT_SUPPORTED;
	syscall(SYS_reboot, KSU_INSTALL_MAGIC1, SUSFS_MAGIC, CMD_SUSFS_ADD_SUS_MAP, &info);
	PRT_MSG_IF_CMD_NOT_SUPPORTED(info.err, CMD_SUSFS_ADD_SUS_MAP);
	return info.err;
}

