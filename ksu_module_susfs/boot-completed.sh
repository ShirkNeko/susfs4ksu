#!/system/bin/sh
PATH=/data/adb/ksu/bin:$PATH

MODDIR=/data/adb/modules/susfs4ksu

SUSFS_BIN=/data/adb/ksu/bin/ksu_susfs

#### When the filter is set, only the listed apps can see the added sus paths ####
## ksu_susfs set_sus_path_uid_filter <app_package_names_separated_by_comma> ##
cat <<EOF >/dev/null
ksu_susfs set_sus_path_uid_filter "com.termux,org.frknkrc44.hma_oss,io.github.vvb2060.keyattestation"
EOF

#### Hide some sus paths from all processes with uid >= 10000 that are not in set_sus_path_uid_filter ####
cat <<EOF >/dev/null
## First we need to wait until files are accessible in /sdcard ##
until [ -d "/sdcard/Android" ]; do sleep 3; done

## Remove the '..5.u.S' leftover ##
## THe reason why this sus file is created is because users have grant the MANAGE_EXTERNAL_STORAGE permission for the apps that detecting sus files in /sdcard, or in /sdcard/Android/data where the apps are exploiting the unicode bugs to create files arbitrary.
## susfs redirects the sus path to a supposed not-existing path named '..5.u.S', and this is the only way to settle the cross check of returned errno from various syscalls, but one disadvantage is that if the path itself can be written/created by the app (MANAGE_EXTERNAL_STORAGE granted), then it is futile to hide it, but at least here we automatically delete them on each boot.
## The best practise is to revoke MANAGE_EXTERNAL_STORAGE permission for all third party apps.

[ -e "/sdcard/..5.u.S" ] && rm -rf "/sdcard/..5.u.S"
[ -e "/sdcard/Android/data/..5.u.S" ] && rm -rf "/sdcard/Android/data/..5.u.S"
[ -e "/sdcard/Android/media/..5.u.S" ] && rm -rf "/sdcard/Android/media/..5.u.S"
[ -e "/sdcard/Android/obb/..5.u.S" ] && rm -rf "/sdcard/Android/obb/..5.u.S"

## For paths that are read-only all the time, add them via 'add_sus_path' ##
${SUSFS_BIN} add_sus_path /sys/block/loop0
${SUSFS_BIN} add_sus_path /system/addon.d
${SUSFS_BIN} add_sus_path /vendor/bin/install-recovery.sh
${SUSFS_BIN} add_sus_path /system/bin/install-recovery.sh

## For paths that are frequently modified, we can add them via 'add_sus_path_loop' ##
## path in /sdcard ##
${SUSFS_BIN} add_sus_path_loop /sdcard/TWRP
${SUSFS_BIN} add_sus_path_loop /sdcard/MT2
${SUSFS_BIN} add_sus_path_loop /sdcard/AppManager
${SUSFS_BIN} add_sus_path_loop /sdcard/Android/data/io.github.muntashirakon.AppManager
${SUSFS_BIN} add_sus_path_loop /sdcard/Android/media/io.github.muntashirakon.AppManager
## Be reminded that without HMA's vold app data enabled, added sus_paths are still vulnerable to zwc exploit, so in this case users also have to add its underlying path as well ##
${SUSFS_BIN} add_sus_path_loop /data/media/0/TWRP
${SUSFS_BIN} add_sus_path_loop /data/media/0/MT2
${SUSFS_BIN} add_sus_path_loop /data/media/0/AppManager
${SUSFS_BIN} add_sus_path_loop /data/media/0/Android/data/io.github.muntashirakon.AppManager
${SUSFS_BIN} add_sus_path_loop /data/media/0/Android/media/io.github.muntashirakon.AppManager
## path not in /sdcard ##
${SUSFS_BIN} add_sus_path_loop /data/local/tmp/main.jar
EOF

#### Hide the mmapped real file from various maps in /proc/self/ ####
cat <<EOF >/dev/null
## - Please note that it is better to do it in boot-completed starge
##   Since some target path may be mounted by ksu, and make sure the
##   target path has the same dev number as the one in global mnt ns,
##   otherwise the sus map flag won't be seen on the umounted process.
## - To debug with this, users can do this in a root shell:
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
ksu_susfs add_sus_map /data/adb/modules/my_module/zygisk/arm64-v8a.so

## Hide some map traces caused by some font modules ##
ksu_susfs add_sus_map /system/fonts/Roboto-Regular.ttf
ksu_susfs add_sus_map /system/fonts/RobotoStatic-Regular.ttf
EOF

#### Unhide all sus mounts from /proc/self/[mounts|mountinfo|mountstat] for non-su processes ####
## It is suggested to unhide it in this stage, and let kernel or zygisk to umount them for user processes, but this is up to you ##
cat <<EOF >/dev/null
ksu_susfs hide_sus_mnts_for_non_su_procs 0
EOF

#### Hide the uids of user-selected apps from the getpriority() syscall ####
## ksu_susfs set_getpriority_uid_filter <app_package_names_separated_by_comma> ##
cat <<EOF >/dev/null
ksu_susfs set_priority_uid_filter "com.termux,org.frknkrc44.hma_oss,io.github.vvb2060.keyattestation"
EOF

