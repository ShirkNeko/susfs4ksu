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

#### Hide path like /sdcard/<target_root_dir> from all user app processes without root access ####
cat <<EOF >/dev/null
## First we need to wait until files are accessible in /sdcard ##
until [ -d "/sdcard/Android" ]; do sleep 1; done

## Next we need to set the path of /sdcard/ to tell kernel where the actual /sdcard is ##
ksu_susfs set_sdcard_root_path /sdcard

## Now we can add the path ##
ksu_susfs add_sus_path /sdcard/TWRP
ksu_susfs add_sus_path /sdcard/MT2

## Please note that sometimes the path needs to be added twice or above to be effective ##
## Besides, all user apps without root access cannot see the hidden path '/sdcard/<hidden_path>' unless you grant it root access ##
EOF

#### Hide the leaking app path like /sdcard/Android/data/<app_package_name> from syscall ####
cat <<EOF >/dev/null
## First we need to wait until files are accessible in /sdcard ##
until [ -d "/sdcard/Android" ]; do sleep 1; done

## Next we need to set the path of /sdcard/ to tell kernel where the actual /sdcard/Android/data is ##
ksu_susfs set_android_data_root_path /sdcard/Android/data

## Now we can add the path ##
ksu_susfs add_sus_path /sdcard/Android/data/bin.mt.plus
EOF

#### For path that needs to be re-flagged as SUS_PATH on each non-root user app / isolated service starts via add_sus_path_loop ####
cat <<EOF >/dev/null
## - Path added via add_sus_path_loop will be re-flagged as SUS_PATH on each non-root process / isolated service starts ##
## - This can help ensure some path that keep its inode status reset for whatever reason to be flagged as SUS_PATH again ##
## - Please also note that only paths NOT inside '/sdcard/' or '/storage/' can be added via add_sus_path_loop ##
## - ONLY USE THIS WHEN NECCESSARY !!! ##
ksu_susfs add_sus_path_loop /sys/block/loop0
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

