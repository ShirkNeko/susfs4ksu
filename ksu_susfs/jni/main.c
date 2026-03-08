#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <susfs_utils.h>
#include <susfs_defs.h>
#include "features.h"

static void print_help(void) {
	print_help_banner();

	sus_path_print_help();
	sus_mount_print_help();
	sus_kstat_print_help();
	sus_map_print_help();
	set_uname_print_help();
	set_cmdline_or_bootconfig_print_help();
	enable_log_print_help();
	enable_avc_log_spoofing_print_help();
	open_redirect_print_help();
	show_print_help();
}

static inline void pre_check(int argc) {
	if (getuid() != 0) {
		log("[-] Must run as root\n");
		exit(-EPERM);
	}

	if (argc < 2) {
		print_help();
		exit(-EINVAL);
	}
}

/*******************
 ** Main Function **
 *******************/
int main(int argc, char *argv[]) {
	pre_check(argc);

	if (!strcmp(argv[1], "add_sus_path"))
		return add_sus_path(argc, argv);
	if (!strcmp(argv[1], "add_sus_path_loop"))
		return add_sus_path_loop(argc, argv);
	if (!strcmp(argv[1], "hide_sus_mnts_for_non_su_procs"))
		return hide_sus_mnts_for_non_su_procs(argc, argv);
	if (!strcmp(argv[1], "add_sus_map"))
		return add_sus_map(argc, argv);
	if (!strcmp(argv[1], "add_open_redirect"))
		return add_open_redirect(argc, argv);
	if (!strcmp(argv[1], "add_sus_kstat_statically"))
		return add_sus_kstat_statically(argc, argv);
	if (!strcmp(argv[1], "add_sus_kstat"))
		return add_sus_kstat(argc, argv);
	if (!strcmp(argv[1], "update_sus_kstat"))
		return update_sus_kstat(argc, argv);
	if (!strcmp(argv[1], "update_sus_kstat_full_clone"))
		return update_sus_kstat_full_clone(argc, argv);
	if (!strcmp(argv[1], "enable_log"))
		return enable_log(argc, argv);
	if (!strcmp(argv[1], "set_cmdline_or_bootconfig"))
		return set_cmdline_or_bootconfig(argc, argv);
	if (!strcmp(argv[1], "enable_avc_log_spoofing"))
		return enable_avc_log_spoofing(argc, argv);
	if (!strcmp(argv[1], "show"))
		return show(argc, argv);
	if (!strcmp(argv[1], "set_uname"))
		return set_uname(argc, argv);
	print_help();
	return -EINVAL;
}

