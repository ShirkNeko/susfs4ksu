#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/reboot.h>
#include <sys/syscall.h>
#include <errno.h>
#include <susfs_defs.h>
#include <susfs_utils.h>
#include "sus_path.h"

#define CMD_SUSFS_ADD_SUS_PATH 0x55550
#define CMD_SUSFS_ADD_SUS_PATH_LOOP 0x55553

struct st_susfs_sus_path {
	char                    target_pathname[SUSFS_MAX_LEN_PATHNAME];
	int                     err;
};

void sus_path_print_help(void){
	log("    add_sus_path </path/of/file_or_directory>\n");
	log("      |--> Added path and all its sub-paths will be hidden for umounted app process from several syscalls\n");
	log("      |--> Please be reminded that if the target path has upper mounts then make sure the proper layer is added, otherwise it may not be effective for the target process.\n");
	log("      * Important Notes *\n");
	log("      - Only effective for umounted process with uid >= 10000\n");
	log("\n");
	log("    add_sus_path_loop </path/of/file_or_directory>\n");
	log("      |--> The only difference to add_sus_path is that the added sus_path via this cli will be flagged as SUS_PATH again for the app process when it is being spawned by zygote and marked umounted\n");
	log("      |--> Also it does not check if the path is existed or not, instead it checks for empty string only, so be careful what to add.\n");
	log("      * Important Notes *\n");
	log("      - Only effective for umounted process with uid >= 10000\n");
	log("\n");
}

static void print_help(void){
	print_help_banner();
	sus_path_print_help();
}

int add_sus_path(int argc, char *argv[]) {
	struct st_susfs_sus_path info = {0};
	struct stat sb;

	if (argc != 3) {
		print_help();
		return -EINVAL;
	}

	info.err = get_file_stat(argv[2], &sb);
	if (info.err) {
		log("[-] failed to get stat from path: '%s'\n", argv[2]);
		return info.err;
	}
	strncpy(info.target_pathname, argv[2], SUSFS_MAX_LEN_PATHNAME-1);
	info.err = ERR_CMD_NOT_SUPPORTED;
	syscall(SYS_reboot, KSU_INSTALL_MAGIC1, SUSFS_MAGIC, CMD_SUSFS_ADD_SUS_PATH, &info);
	PRT_MSG_IF_CMD_NOT_SUPPORTED(info.err, CMD_SUSFS_ADD_SUS_PATH);
	return info.err;
}

int add_sus_path_loop(int argc, char *argv[]) {
	struct st_susfs_sus_path info = {0};
	struct stat sb = {0};

	if (argc != 3) {
		print_help();
		return -EINVAL;
	}

	if (*argv[2] == '\0') {
		log("[-] argv[2] is empty'\n");
		return -EINVAL;
	}

	strncpy(info.target_pathname, argv[2], SUSFS_MAX_LEN_PATHNAME-1);
	info.err = ERR_CMD_NOT_SUPPORTED;
	syscall(SYS_reboot, KSU_INSTALL_MAGIC1, SUSFS_MAGIC, CMD_SUSFS_ADD_SUS_PATH_LOOP, &info);
	PRT_MSG_IF_CMD_NOT_SUPPORTED(info.err, CMD_SUSFS_ADD_SUS_PATH_LOOP);
	return info.err;
}
