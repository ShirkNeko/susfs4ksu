#ifndef SUS_KSTAT_H
#define SUS_KSTAT_H

void sus_kstat_print_help(void);
int add_sus_kstat_statically(int argc, char *argv[]);
int add_sus_kstat(int argc, char *argv[]);
int update_sus_kstat(int argc, char *argv[]);
int update_sus_kstat_full_clone(int argc, char *argv[]);

#endif // #ifndef SUS_KSTAT_H
