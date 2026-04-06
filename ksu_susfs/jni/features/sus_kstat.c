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
#include "sus_kstat.h"

#define CMD_SUSFS_ADD_SUS_KSTAT 0x55570
#define CMD_SUSFS_UPDATE_SUS_KSTAT 0x55571
#define CMD_SUSFS_ADD_SUS_KSTAT_STATICALLY 0x55572

#define KSTAT_SPOOF_INO (1 << 0)
#define KSTAT_SPOOF_DEV (1 << 1)
#define KSTAT_SPOOF_NLINK (1 << 2)
#define KSTAT_SPOOF_SIZE (1 << 3)
#define KSTAT_SPOOF_ATIME_TV_SEC (1 << 4)
#define KSTAT_SPOOF_ATIME_TV_NSEC (1 << 5)
#define KSTAT_SPOOF_MTIME_TV_SEC (1 << 6)
#define KSTAT_SPOOF_MTIME_TV_NSEC (1 << 7)
#define KSTAT_SPOOF_CTIME_TV_SEC (1 < 8)
#define KSTAT_SPOOF_CTIME_TV_NSEC (1 << 9)
#define KSTAT_SPOOF_BLOCKS (1 << 10)
#define KSTAT_SPOOF_BLKSIZE (1 << 11)
#define KSTAT_AUTO_SPOOF (KSTAT_SPOOF_INO | KSTAT_SPOOF_DEV | KSTAT_SPOOF_ATIME_TV_SEC | KSTAT_SPOOF_ATIME_TV_NSEC | \
		    KSTAT_SPOOF_MTIME_TV_SEC | KSTAT_SPOOF_MTIME_TV_NSEC | KSTAT_SPOOF_CTIME_TV_SEC | KSTAT_SPOOF_CTIME_TV_NSEC | \
		    KSTAT_SPOOF_BLKSIZE | KSTAT_SPOOF_BLOCKS)
#define KSTAT_AUTO_SPOOF_FULL_CLONE (KSTAT_AUTO_SPOOF | KSTAT_SPOOF_NLINK | KSTAT_SPOOF_SIZE)

struct st_susfs_sus_kstat {
	bool                    is_statically;
	unsigned long           target_ino;
	char                    target_pathname[SUSFS_MAX_LEN_PATHNAME];
	unsigned long           spoofed_ino;
	unsigned long           spoofed_dev;
	unsigned int            spoofed_nlink;
	long long               spoofed_size;
	long                    spoofed_atime_tv_sec;
	unsigned long           spoofed_atime_tv_nsec;
	long                    spoofed_mtime_tv_sec;
	unsigned long           spoofed_mtime_tv_nsec;
	long                    spoofed_ctime_tv_sec;
	unsigned long           spoofed_ctime_tv_nsec;
	long long               spoofed_blocks;
	long                    spoofed_blksize;
	int                     flags;
	int                     err;
};

static void copy_from_stat_to_sus_kstat(struct st_susfs_sus_kstat* info, struct stat* sb) {
	info->spoofed_ino = sb->st_ino;
	info->spoofed_dev = sb->st_dev;
	info->spoofed_nlink = sb->st_nlink;
	info->spoofed_size = sb->st_size;
	info->spoofed_atime_tv_sec = sb->st_atime;
	info->spoofed_atime_tv_nsec = sb->st_atime_nsec;
	info->spoofed_mtime_tv_sec = sb->st_mtime;
	info->spoofed_mtime_tv_nsec = sb->st_mtime_nsec;
	info->spoofed_ctime_tv_sec = sb->st_ctime;
	info->spoofed_ctime_tv_nsec = sb->st_ctime_nsec;
	info->spoofed_blksize = sb->st_blksize;
	info->spoofed_blocks = sb->st_blocks;
}

void sus_kstat_print_help(void){
	log("    add_sus_kstat_statically </path/of/file_or_directory> <ino> <dev> <nlink> <size> <atime> <atime_nsec> <mtime> <mtime_nsec> <ctime> <ctime_nsec> <blocks> <blksize>\n");
	log("      |--> Use 'stat' tool to find the format:\n");
	log("               ino -> %%i, dev -> %%d, nlink -> %%h, atime -> %%X, mtime -> %%Y, ctime -> %%Z\n");
	log("               size -> %%s, blocks -> %%b, blksize -> %%B\n");
	log("      |--> e.g., %s add_sus_kstat_statically '/system/addon.d' '1234' '1234' '2' '223344'\\\n", TAG);
	log("                    '1712592355' '0' '1712592355' '0' '1712592355' '0' '1712592355' '0'\\\n");
	log("                    '16' '512'\n");
	log("      |--> Or pass 'default' to use its original value:\n");
	log("      |--> e.g., %s add_sus_kstat_statically '/system/addon.d' 'default' 'default' 'default' 'default'\\\n", TAG);
	log("                    '1712592355' 'default' '1712592355' 'default' '1712592355' 'default'\\\n");
	log("                    'default' 'default'\n");
	log("      * Important Notes *\n");
	log("      - Only effective for umounted process with uid >= 10000\n");
	log("\n");
	log("    add_sus_kstat </path/of/file_or_directory>\n");
	log("      |--> Add the desired path BEFORE it gets bind mounted or overlayed, this is used for storing original stat info in kernel memory\n");
	log("      |--> This command must be completed with <update_sus_kstat> later after the added path is bind mounted or overlayed\n");
	log("      * Important Notes *\n");
	log("      - Only effective for umounted process with uid >= 10000\n");
	log("\n");
	log("    update_sus_kstat </path/of/file_or_directory>\n");
	log("      |--> Add the desired path you have added before via <add_sus_kstat> to complete the kstat spoofing procedure\n");
	log("      |--> This updates the target ino, but size and blocks are remained the same as current stat\n");
	log("      * Important Notes *\n");
	log("      - Only effective for umounted process with uid >= 10000\n");
	log("\n");
	log("    update_sus_kstat_full_clone </path/of/file_or_directory>\n");
	log("      |--> Add the desired path you have added before via <add_sus_kstat> to complete the kstat spoofing procedure\n");
	log("      |--> This updates the target ino only, other stat members are remained the same as the original stat\n");
	log("      * Important Notes *\n");
	log("      - Only effective for umounted process with uid >= 10000\n");
	log("\n");
}

static void print_help(void){
	print_help_banner();
	sus_kstat_print_help();
}

int add_sus_kstat_statically(int argc, char *argv[]) {
	struct st_susfs_sus_kstat info = {0};
	struct stat sb;
	char resolved_pathname[PATH_MAX];
	char* endptr;
	unsigned long ino, dev, nlink, size, atime, atime_nsec, mtime, mtime_nsec, ctime, ctime_nsec, blksize;
	long blocks;

	if (argc != 15) {
		print_help();
		return -EINVAL;
	}

	if (*argv[2] == '\0') {
		log("[-] argv[2] is empty'\n");
		return -EINVAL;
	}

	if (!realpath(argv[2], resolved_pathname)) {
		log("[-] failed to get realpath from path: %s\n", argv[2]);
		return errno;
	}

	/* get the stat of the target path first */
	info.err = get_file_stat(resolved_pathname, &sb);
	if (info.err) {
		log("[-] failed to get stat from path: '%s'\n", resolved_pathname);
		return info.err;
	}
	/* it is statically */
	info.is_statically = true;
	info.target_ino = sb.st_ino;

	/* ino */
	if (strcmp(argv[3], "default")) {
		ino = strtoul(argv[3], &endptr, 10);
		if (*endptr != '\0') {
			print_help();
			return -EINVAL;
		}
		sb.st_ino = ino;
		info.flags |= KSTAT_SPOOF_INO;
	}
	/* dev */
	if (strcmp(argv[4], "default")) {
		dev = strtoul(argv[4], &endptr, 10);
		if (*endptr != '\0') {
			print_help();
			return -EINVAL;
		}
		sb.st_dev = dev;
		info.flags |= KSTAT_SPOOF_DEV;
	}
	/* nlink */
	if (strcmp(argv[5], "default")) {
		nlink = strtoul(argv[5], &endptr, 10);
		if (*endptr != '\0') {
			print_help();
			return -EINVAL;
		}
		sb.st_nlink = nlink;
		info.flags |= KSTAT_SPOOF_NLINK;
	}
	/* size */
	if (strcmp(argv[6], "default")) {
		size = strtoul(argv[6], &endptr, 10);
		if (*endptr != '\0') {
			print_help();
			return -EINVAL;
		}
		sb.st_size = size;
		info.flags |= KSTAT_SPOOF_SIZE;
	}
	/* atime */
	if (strcmp(argv[7], "default")) {
		atime = strtol(argv[7], &endptr, 10);
		if (*endptr != '\0') {
			print_help();
			return -EINVAL;
		}
		sb.st_atime = atime;
		info.flags |= KSTAT_SPOOF_ATIME_TV_SEC;
	}
	/* atime_nsec */
	if (strcmp(argv[8], "default")) {
		atime_nsec = strtoul(argv[8], &endptr, 10);
		if (*endptr != '\0') {
			print_help();
			return -EINVAL;
		}
		sb.st_atimensec = atime_nsec;
		info.flags |= KSTAT_SPOOF_ATIME_TV_NSEC;
	}
	/* mtime */
	if (strcmp(argv[9], "default")) {
		mtime = strtol(argv[9], &endptr, 10);
		if (*endptr != '\0') {
			print_help();
			return -EINVAL;
		}
		sb.st_mtime = mtime;
		info.flags |= KSTAT_SPOOF_MTIME_TV_SEC;
	}
	/* mtime_nsec */
	if (strcmp(argv[10], "default")) {
		mtime_nsec = strtoul(argv[10], &endptr, 10);
		if (*endptr != '\0') {
			print_help();
			return -EINVAL;
		}
		sb.st_mtimensec = mtime_nsec;
		info.flags |= KSTAT_SPOOF_MTIME_TV_NSEC;
	}
	/* ctime */
	if (strcmp(argv[11], "default")) {
		ctime = strtol(argv[11], &endptr, 10);
		if (*endptr != '\0') {
			print_help();
			return -EINVAL;
		}
		sb.st_ctime = ctime;
		info.flags |= KSTAT_SPOOF_CTIME_TV_SEC;
	}
	/* ctime_nsec */
	if (strcmp(argv[12], "default")) {
		ctime_nsec = strtoul(argv[12], &endptr, 10);
		if (*endptr != '\0') {
			print_help();
			return -EINVAL;
		}
		sb.st_ctimensec = ctime_nsec;
		info.flags |= KSTAT_SPOOF_CTIME_TV_NSEC;
	}
	/* blocks */
	if (strcmp(argv[13], "default")) {
		blocks = strtoul(argv[13], &endptr, 10);
		if (*endptr != '\0') {
			print_help();
			return -EINVAL;
		}
		sb.st_blocks = blocks;
		info.flags |= KSTAT_SPOOF_BLOCKS;
	}
	/* blksize */
	if (strcmp(argv[14], "default")) {
		blksize = strtoul(argv[14], &endptr, 10);
		if (*endptr != '\0') {
			print_help();
			return -EINVAL;
		}
		sb.st_blksize = blksize;
		info.flags |= KSTAT_SPOOF_BLKSIZE;
	}

	strncpy(info.target_pathname, resolved_pathname, SUSFS_MAX_LEN_PATHNAME-1);
	copy_from_stat_to_sus_kstat(&info, &sb);
	info.err = ERR_CMD_NOT_SUPPORTED;
	syscall(SYS_reboot, KSU_INSTALL_MAGIC1, SUSFS_MAGIC, CMD_SUSFS_ADD_SUS_KSTAT_STATICALLY, &info);
	PRT_MSG_IF_CMD_NOT_SUPPORTED(info.err, CMD_SUSFS_ADD_SUS_KSTAT_STATICALLY);
	return info.err;
}

int add_sus_kstat(int argc, char *argv[]) {
	struct st_susfs_sus_kstat info = {0};
	struct stat sb;
	char resolved_pathname[PATH_MAX];

	if (argc != 3) {
		print_help();
		return -EINVAL;
	}

	if (*argv[2] == '\0') {
		log("[-] argv[2] is empty'\n");
		return -EINVAL;
	}

	if (!realpath(argv[2], resolved_pathname)) {
		log("[-] failed to get realpath from path: %s\n", argv[2]);
		return errno;
	}

	info.err = get_file_stat(resolved_pathname, &sb);
	if (info.err) {
		log("[-] failed to get stat from path: '%s'\n", resolved_pathname);
		return info.err;
	}
	strncpy(info.target_pathname, resolved_pathname, SUSFS_MAX_LEN_PATHNAME-1);
	info.is_statically = false;
	info.target_ino = sb.st_ino;
	info.flags |= KSTAT_AUTO_SPOOF;
	copy_from_stat_to_sus_kstat(&info, &sb);
	info.err = ERR_CMD_NOT_SUPPORTED;
	syscall(SYS_reboot, KSU_INSTALL_MAGIC1, SUSFS_MAGIC, CMD_SUSFS_ADD_SUS_KSTAT, &info);
	PRT_MSG_IF_CMD_NOT_SUPPORTED(info.err, CMD_SUSFS_ADD_SUS_KSTAT);
	return info.err;
}

int update_sus_kstat(int argc, char *argv[]) {
	struct st_susfs_sus_kstat info = {0};
	struct stat sb;
	char resolved_pathname[PATH_MAX];

	if (argc != 3) {
		print_help();
		return -EINVAL;
	}

	if (*argv[2] == '\0') {
		log("[-] argv[2] is empty'\n");
		return -EINVAL;
	}

	if (!realpath(argv[2], resolved_pathname)) {
		log("[-] failed to get realpath from path: %s\n", argv[2]);
		return errno;
	}

	info.err = get_file_stat(resolved_pathname, &sb);
	if (info.err) {
		log("[-] failed to get stat from path: '%s'\n", resolved_pathname);
		return info.err;
	}
	strncpy(info.target_pathname, resolved_pathname, SUSFS_MAX_LEN_PATHNAME-1);
	info.is_statically = false;
	info.target_ino = sb.st_ino;
	info.flags |= KSTAT_AUTO_SPOOF;
	copy_from_stat_to_sus_kstat(&info, &sb);
	info.err = ERR_CMD_NOT_SUPPORTED;
	syscall(SYS_reboot, KSU_INSTALL_MAGIC1, SUSFS_MAGIC, CMD_SUSFS_UPDATE_SUS_KSTAT, &info);
	PRT_MSG_IF_CMD_NOT_SUPPORTED(info.err, CMD_SUSFS_UPDATE_SUS_KSTAT);
	return info.err;
}

int update_sus_kstat_full_clone(int argc, char *argv[]) {
	struct st_susfs_sus_kstat info = {0};
	struct stat sb;
	char resolved_pathname[PATH_MAX];

	if (argc != 3) {
		print_help();
		return -EINVAL;
	}

	if (*argv[2] == '\0') {
		log("[-] argv[2] is empty'\n");
		return -EINVAL;
	}

	if (!realpath(argv[2], resolved_pathname)) {
		log("[-] failed to get realpath from path: %s\n", argv[2]);
		return errno;
	}

	info.err = get_file_stat(resolved_pathname, &sb);
	if (info.err) {
		log("[-] failed to get stat from path: '%s'\n", resolved_pathname);
		return info.err;
	}
	strncpy(info.target_pathname, resolved_pathname, SUSFS_MAX_LEN_PATHNAME-1);
	info.is_statically = false;
	info.target_ino = sb.st_ino;
	info.flags |= KSTAT_AUTO_SPOOF_FULL_CLONE;
	copy_from_stat_to_sus_kstat(&info, &sb);
	info.err = ERR_CMD_NOT_SUPPORTED;
	syscall(SYS_reboot, KSU_INSTALL_MAGIC1, SUSFS_MAGIC, CMD_SUSFS_UPDATE_SUS_KSTAT, &info);
	PRT_MSG_IF_CMD_NOT_SUPPORTED(info.err, CMD_SUSFS_UPDATE_SUS_KSTAT);
	return info.err;
}

