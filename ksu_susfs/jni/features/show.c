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
#include "show.h"

#define CMD_SUSFS_SHOW_VERSION 0x555e1
#define CMD_SUSFS_SHOW_ENABLED_FEATURES 0x555e2
#define CMD_SUSFS_SHOW_VARIANT 0x555e3

#define SUSFS_ENABLED_FEATURES_SIZE 8192
#define SUSFS_MAX_VERSION_BUFSIZE 16
#define SUSFS_MAX_VARIANT_BUFSIZE 16

struct st_susfs_enabled_features {
	char                    enabled_features[SUSFS_ENABLED_FEATURES_SIZE];
	int                     err;
};

struct st_susfs_variant {
	char                    susfs_variant[SUSFS_MAX_VARIANT_BUFSIZE];
	int                     err;
};

struct st_susfs_version {
	char                    susfs_version[SUSFS_MAX_VERSION_BUFSIZE];
	int                     err;
};

void show_print_help(void){
	log("    show <version|enabled_features|variant>\n");
	log("      |--> version: show the current susfs version implemented in kernel\n");
	log("      |--> enabled_features: show the current implemented susfs features in kernel\n");
	log("      |--> variant: show the current variant: GKI or NON-GKI\n");
	log("\n");
}

static void print_help(void){
	print_help_banner();
	show_print_help();
}

int show(int argc, char *argv[]) {
	if (argc != 3) {
		print_help();
		return -EINVAL;
	}

	if (!strcmp(argv[2], "version")) {
		struct st_susfs_version info = {0};

		info.err = ERR_CMD_NOT_SUPPORTED;
		syscall(SYS_reboot, KSU_INSTALL_MAGIC1, SUSFS_MAGIC, CMD_SUSFS_SHOW_VERSION, &info);
		PRT_MSG_IF_CMD_NOT_SUPPORTED(info.err, CMD_SUSFS_SHOW_VERSION);
		if (!info.err)
			log("%s\n", info.susfs_version);
		return info.err;
	} else if (!strcmp(argv[2], "enabled_features")) {
		struct st_susfs_enabled_features *info = malloc(sizeof(struct st_susfs_enabled_features));
		int err = 0;

		if (!info) {
			perror("malloc");
			return -ENOMEM;
		}
		memset(info, 0, sizeof(struct st_susfs_enabled_features));
		info->err = ERR_CMD_NOT_SUPPORTED;
		syscall(SYS_reboot, KSU_INSTALL_MAGIC1, SUSFS_MAGIC, CMD_SUSFS_SHOW_ENABLED_FEATURES, info);
		PRT_MSG_IF_CMD_NOT_SUPPORTED(info->err, CMD_SUSFS_SHOW_ENABLED_FEATURES);
		if (!info->err) {
			log("%s", info->enabled_features); // no new line is needed
		}
		err = info->err;
		free(info);
		return err;
	} else if (!strcmp(argv[2], "variant")) {
		struct st_susfs_variant info = {0};

		info.err = ERR_CMD_NOT_SUPPORTED;
		syscall(SYS_reboot, KSU_INSTALL_MAGIC1, SUSFS_MAGIC, CMD_SUSFS_SHOW_VARIANT, &info);
		PRT_MSG_IF_CMD_NOT_SUPPORTED(info.err, CMD_SUSFS_SHOW_VARIANT);
		if (!info.err)
			log("%s\n", info.susfs_variant);
		return info.err;
	} else {
		print_help();
	}
	return -EINVAL;
}

