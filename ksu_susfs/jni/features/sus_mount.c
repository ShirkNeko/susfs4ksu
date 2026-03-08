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
#include "sus_mount.h"

#define CMD_SUSFS_HIDE_SUS_MNTS_FOR_NON_SU_PROCS 0x55561

struct st_susfs_hide_sus_mnts_for_non_su_procs {
	bool                    enabled;
	int                     err;
};

void sus_mount_print_help(void){
	log("    hide_sus_mnts_for_non_su_procs <0|1>\n");
	log("      |--> 0 -> DO NOT hide sus mounts for non-su processes\n");
	log("      |--> 1 -> hide all sus mounts for non-su processes\n");
	log("      * Important Notes *\n");
	log("      - It is set to 0 in kernel by default\n");
	log("      - For ReZygisk without TreatWheel module, it is recommended to set to 1 in post-fs-data.sh to prevent zygote from caching the sus mounts in memory, and revert to 0 in boot-completed.sh stage, or keep it enabled if you want to keep them hidden from /proc/self/[mounts|mountinfo|mountstat] for non-su processes\n");
	log("\n");
}

static void print_help(void){
	print_help_banner();
	sus_mount_print_help();
}

int hide_sus_mnts_for_non_su_procs(int argc, char *argv[]) {
	struct st_susfs_hide_sus_mnts_for_non_su_procs info = {0};

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
	syscall(SYS_reboot, KSU_INSTALL_MAGIC1, SUSFS_MAGIC, CMD_SUSFS_HIDE_SUS_MNTS_FOR_NON_SU_PROCS, &info);
	PRT_MSG_IF_CMD_NOT_SUPPORTED(info.err, CMD_SUSFS_HIDE_SUS_MNTS_FOR_NON_SU_PROCS);
	return info.err;
}

