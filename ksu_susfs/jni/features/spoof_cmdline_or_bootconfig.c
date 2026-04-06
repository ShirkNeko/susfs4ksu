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

#define CMD_SUSFS_SET_CMDLINE_OR_BOOTCONFIG 0x555b0

#define SUSFS_FAKE_CMDLINE_OR_BOOTCONFIG_SIZE 8192

struct st_susfs_spoof_cmdline_or_bootconfig {
	char                    fake_cmdline_or_bootconfig[SUSFS_FAKE_CMDLINE_OR_BOOTCONFIG_SIZE];
	int                     err;
};

void set_cmdline_or_bootconfig_print_help(void){
	log("    set_cmdline_or_bootconfig </path/to/fake_cmdline_file/or/fake_bootconfig_file>\n");
	log("      |--> Spoof the output of /proc/cmdline (non-gki) or /proc/bootconfig (gki) from a text file\n");
	log("\n");
}

static void print_help(void){
	print_help_banner();
	set_cmdline_or_bootconfig_print_help();
}

int set_cmdline_or_bootconfig(int argc, char *argv[]) {
	struct st_susfs_spoof_cmdline_or_bootconfig *info = malloc(sizeof(struct st_susfs_spoof_cmdline_or_bootconfig));
	char resolved_pathname[PATH_MAX];
	FILE *file;
	long file_size;
	size_t read_size; 
	int err = 0;

	if (argc != 3) {
		print_help();
		return -EINVAL;
	}

	if (!info) {
		perror("malloc");
		return -ENOMEM;
	}

	if (*argv[2] == '\0') {
		log("[-] argv[2] is empty'\n");
		return -EINVAL;
	}

	memset(info, 0, sizeof(struct st_susfs_spoof_cmdline_or_bootconfig));
	if (!realpath(argv[2], resolved_pathname)) {
		perror("realpath");
		free(info);
		return errno;
	}
	file = fopen(resolved_pathname, "rb");
	if (file == NULL) {
		perror("error opening file");
		free(info);
		return errno;
	}
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	if (file_size >= SUSFS_FAKE_CMDLINE_OR_BOOTCONFIG_SIZE) {
		perror("file_size too long");
		free(info);
		return -EINVAL;
	}
	rewind(file);
	read_size = fread(info->fake_cmdline_or_bootconfig, 1, file_size, file);
	if (read_size != file_size) {
		perror("reading error");
		fclose(file);
		free(info);
		return -EFAULT;
	}
	fclose(file);
	info->fake_cmdline_or_bootconfig[file_size] = '\0';
	info->err = ERR_CMD_NOT_SUPPORTED;
	syscall(SYS_reboot, KSU_INSTALL_MAGIC1, SUSFS_MAGIC, CMD_SUSFS_SET_CMDLINE_OR_BOOTCONFIG, info);
	PRT_MSG_IF_CMD_NOT_SUPPORTED(info->err, CMD_SUSFS_SET_CMDLINE_OR_BOOTCONFIG);
	err = info->err;
	free(info);
	return err;
}

