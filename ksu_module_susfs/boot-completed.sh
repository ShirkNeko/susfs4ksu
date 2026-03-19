#!/system/bin/sh
PATH=/data/adb/ksu/bin:$PATH

MODDIR=/data/adb/modules/susfs4ksu

SUSFS_BIN=/data/adb/ksu/bin/ksu_susfs

#### Adding sus mounts to umount list via built-in KernelSU kernel umount (not via add_try_umount from old susfs) ####
cat <<EOF >/dev/null
## Don't forget to notify KernelSU that all ksu modules all mounted and ready ##
/data/adb/ksu/bin/ksud kernel notify-module-mounted

## This is just an example to add the sus mounts to kernel umount ##
if [ ! -f "/data/adb/susfs_no_auto_add_kernel_umount" ]; then
	cat /proc/1/mountinfo | grep -E "^5[0-9]{5,} .*$|KSU" | awk '{print $5}' | while read -r LINE; do /data/adb/ksu/bin/ksud kernel umount add --flags 2 "${LINE}" 2>/dev/null; done
fi
EOF

#### Hide some sus paths, effective only for processes that are marked umounted with uid >= 10000 ####
cat <<EOF >/dev/null
## First we need to wait until files are accessible in /sdcard ##
until [ -d "/sdcard/Android" ]; do sleep 1; done

## For paths that are read-only all the time, add them via 'add_sus_path' ##
${SUSFS_BIN} add_sus_path /sys/block/loop0
${SUSFS_BIN} add_sus_path /system/addon.d
${SUSFS_BIN} add_sus_path /vendor/bin/install-recovery.sh
${SUSFS_BIN} add_sus_path /system/bin/install-recovery.sh

## For paths that are frequently modified, we can add them via 'add_sus_path_loop' ##
${SUSFS_BIN} add_sus_path_loop /sdcard/TWRP
${SUSFS_BIN} add_sus_path_loop /sdcard/MT2
${SUSFS_BIN} add_sus_path_loop /sdcard/AppManager
${SUSFS_BIN} add_sus_path_loop /sdcard/Android/data/io.github.muntashirakon.AppManager
${SUSFS_BIN} add_sus_path_loop /sdcard/Android/media/io.github.muntashirakon.AppManager
${SUSFS_BIN} add_sus_path_loop /data/local/tmp/main.jar
EOF


#### Hide the mmapped real file from various maps in /proc/self/, effective only for processes that are marked umounted with uid >= 10000 ####
cat <<EOF >/dev/null
## - *Please note that it is better to do it in boot-completed starge
##   Since some target path may be mounted by ksu, and make sure the
##   target path has the same dev number as the one in global mnt ns,
##   otherwise the sus map flag won't be seen on the umounted proocess.
## - *Besides, if the source files get umounted and stay only in like zygote's memory maps,
##   then it will not work as well since sus_map checks for real file's inode.
## - To debug the namespace issue, users can do this in a root shell:
##   1. Find the pid and uid of a opened umounted app by running
##      ps -enf | grep myapp
##   2. cat /proc/<pid_of_myapp>/maps | grep "<added/sus_map/path>"'
##   3. In other root shell, run
##      cat /proc/1/mountinfo | grep "<added/sus_map/path>"'
##   4. Finally compare the dev number with both output and see if they are consistent,
##      if so, then it should be working, but if not, then the added sus_map path
##      is probably not working, and you have to find out which mnt ns the dev number
##      from step 2 belongs to, and add the path from that mnt ns:
##         busybox nsenter -t <pid_of_mnt_ns_the_target_dev_number_belongs_to> -m ksu_susfs add_sus_map <target_path>

## Hide some zygisk modules ##
${SUSFS_BIN} add_sus_map /data/adb/modules/my_module/zygisk/arm64-v8a.so

## Hide some map traces caused by some font modules ##
${SUSFS_BIN} add_sus_map /system/fonts/Roboto-Regular.ttf
${SUSFS_BIN} add_sus_map /system/fonts/RobotoStatic-Regular.ttf
EOF

#### Unhide all sus mounts from /proc/self/[mounts|mountinfo|mountstat] for NON-SU processes ####
## It is suggested to unhide it in this stage, and let kernel or zygisk to umount them for user processes, but this is up to you ##
cat <<EOF >/dev/null
${SUSFS_BIN} hide_sus_mnts_for_non_su_procs 0
EOF

