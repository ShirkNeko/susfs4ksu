#!/system/bin/sh
PATH=/data/adb/ksu/bin:$PATH

MODDIR=/data/adb/modules/susfs4ksu

SUSFS_BIN=/data/adb/ksu/bin/ksu_susfs

source ${MODDIR}/utils.sh

## Important Notes:
## - The following command can be run at other stages like service.sh, boot-completed.sh etc..,
## - This module is just an demo showing how to use ksu_susfs tool to commuicate with kernel
##
## - for add_sus_path and add_sus_kstat if target file/dir is re-created or its ino number is changed, you have to re-run the same susfs command
## - for add_sus_mount, if target path is umounted globally, you have to re-run the same susfs command
##
## - add_sus_path, add_sus_kstat, add_sus_mount and set_uname are allowed to be updated with new spoofed values
##   that said, you can run multiple times with same target input but with different spoofed values.
##
## - BE CAUTIOUS that add_sus_path, add_sus_mount must be added after overlay or mounted operations on target files/direcotories,
##   because susfs now mainly compare target's inode to determine whether it is a sus path, sus kstat or sus mount.
##

#### Hide target path and all its sub-paths from all user app processes which have no root permission granted ####
## Make sure the target file/directory has no more overlay/mount operation on going. Or add it after it is done being overlayed or mounted ##
# For some custom ROM #
cat <<EOF >/dev/null
${SUSFS_BIN} add_sus_path /system/addon.d
${SUSFS_BIN} add_sus_path /vendor/bin/install-recovery.sh
${SUSFS_BIN} add_sus_path /system/bin/install-recovery.sh
EOF

#### Hide the mounted path in /proc/self/[mounts|mountstat|mountinfo] for all processes ####
## Make sure the target file/directory has no more overlay/mount operation on going. Or add it after it is done being overlayed or mounted ##
cat <<EOF >/dev/null
# for default ksu mounts #
${SUSFS_BIN} add_sus_mount /data/adb/modules
${SUSFS_BIN} add_sus_mount /debug_ramdisk
# for host file #
${SUSFS_BIN} add_sus_mount /system/etc/hosts
## for lsposed, choose those that show up in your mountinfo, no need to add them all ##
# also somehow you may need to add the source path to sus mount as well like below #
${SUSFS_BIN} add_sus_mount /data/adb/modules/zygisk_lsposed/bin/dex2oat
${SUSFS_BIN} add_sus_mount /data/adb/modules/zygisk_lsposed/bin/dex2oat32
${SUSFS_BIN} add_sus_mount /data/adb/modules/zygisk_lsposed/bin/dex2oat64
${SUSFS_BIN} add_sus_mount /system/apex/com.android.art/bin/dex2oat
${SUSFS_BIN} add_sus_mount /system/apex/com.android.art/bin/dex2oat32
${SUSFS_BIN} add_sus_mount /system/apex/com.android.art/bin/dex2oat64
${SUSFS_BIN} add_sus_mount /apex/com.android.art/bin/dex2oat
${SUSFS_BIN} add_sus_mount /apex/com.android.art/bin/dex2oat32
${SUSFS_BIN} add_sus_mount /apex/com.android.art/bin/dex2oat64
EOF

#### Umount the mounted path for no root granted process ####
# Please be reminded that susfs's try_umount takes precedence of ksu's try_umount #
cat <<EOF >/dev/null
# for /system/etc/hosts #
${SUSFS_BIN} add_try_umount /system/etc/hosts 1
# for lsposed, choose those that show up in your mountinfo, no need to add them all #
${SUSFS_BIN} add_try_umount /system/apex/com.android.art/bin/dex2oat 1
${SUSFS_BIN} add_try_umount /system/apex/com.android.art/bin/dex2oat32 1
${SUSFS_BIN} add_try_umount /system/apex/com.android.art/bin/dex2oat64 1
${SUSFS_BIN} add_try_umount /apex/com.android.art/bin/dex2oat 1
${SUSFS_BIN} add_try_umount /apex/com.android.art/bin/dex2oat32 1
${SUSFS_BIN} add_try_umount /apex/com.android.art/bin/dex2oat64 1
EOF

#### Spoof the stat of file/directory dynamically ####
## Important Note: 
##  - All paths set with sus_kstat and bind mounted will not be added to try_umount list
##  - It is stronly suggested to use dynamically if the target path will be mounted
cat <<EOF >/dev/null
# First, clone the permission before adding to sus_kstat
susfs_clone_perm "$MODDIR/hosts" /system/etc/hosts

# Second, before bind mount your file/directory, use 'add_sus_kstat' to add the path #
${SUSFS_BIN} add_sus_kstat '/system/etc/hosts'

# Now bind mount or overlay your path #
mount -o bind "$MODDIR/hosts" /system/etc/hosts

# Finally use 'update_sus_kstat' to update the path again for the changed ino and device number #
# update_sus_kstat updates ino, but blocks and size are remained the same as current stat #
${SUSFS_BIN} update_sus_kstat '/system/etc/hosts'

# Or if you want to fully clone the stat value from the original stat, use update_sus_kstat_full_clone instead #
#${SUSFS_BIN} update_sus_kstat_full_clone '/system/etc/hosts'
EOF

#### Spoof the stat of file/directory statically ####
## Important Note:
##  - It is suggested to use statically if you don't need to mount anything but simply change the stat of a target path
cat <<EOF >/dev/null
Usage: ksu_susfs add_sus_kstat_statically </path/of/file_or_directory> \
                        <ino> <dev> <nlink> <size> <atime> <atime_nsec> <mtime> <mtime_nsec> <ctime> <ctime_nsec> \
                        <blocks> <blksize>
${SUSFS_BIN} add_sus_kstat_statically '/system/framework/services.jar' 'default' 'default' 'default' 'default' '1230768000' '0' '1230768000' '0' '1230768000' '0' 'default' 'default'
EOF

#### Spoof the uname ####
# you can get your uname args by running 'uname {-r|-v}' on your stock ROM #
# pass 'default' to tell susfs to use the default value by uname #
cat <<EOF >/dev/null
${SUSFS_BIN} set_uname '5.15.137-android14-11-gb572b1fac135-ab11919372' '#1 SMP PREEMPT Mon Jun 3 16:35:10 UTC 2024'
EOF

#### Redirect path  ####
# redirect hosts file to other hosts file somewhere else #
cat <<EOF >/dev/null
# plesae be reminded that only process with uid < 2000 is effective #
# and before doing that, make sure you setup proper permission and selinux for your redirected file #
susfs_clone_perm '/data/local/tmp/my_hosts' '/system/etc/hosts'
${SUSFS_BIN} add_path_redirect '/system/etc/hosts' '/data/local/tmp/my_hosts'
EOF

#### Spoof /proc/cmdline or /proc/bootconfig ####
# No root process detects it for now, and this spoofing won't help much actually #
# /proc/bootconfig #
cat <<EOF >/dev/null
FAKE_BOOTCONFIG=${MODDIR}/fake_bootconfig.txt
cat /proc/bootconfig > ./fake_bootconfig.txt
sed -i 's/^androidboot.bootreason.*$/androidboot.bootreason = "reboot"/g' ${FAKE_BOOTCONFIG}
sed -i 's/^androidboot.vbmeta.device_state.*$/androidboot.vbmeta.device_state = "locked"/g' ${FAKE_BOOTCONFIG}
sed -i 's/^androidboot.verifiedbootstate.*$/androidboot.verifiedbootstate = "green"/g' ${FAKE_BOOTCONFIG}
sed -i '/androidboot.verifiedbooterror/d' ${FAKE_BOOTCONFIG}
sed -i '/androidboot.verifyerrorpart/d' ${FAKE_BOOTCONFIG}
${SUSFS_BIN} set_cmdline_or_bootconfig /data/adb/modules/susfs4ksu/fake_bootconfig.txt
EOF

# /proc/cmdline #
cat <<EOF >/dev/null
FAKE_PROC_CMDLINE_FILE=${MODDIR}/fake_proc_cmdline.txt
cat /proc/cmdline > ${FAKE_PROC_CMDLINE_FILE}
sed -i 's/androidboot.verifiedbootstate=orange/androidboot.verifiedbootstate=green/g' ${FAKE_PROC_CMDLINE_FILE}
sed -i 's/androidboot.vbmeta.device_state=unlocked/androidboot.vbmeta.device_state=locked/g' ${FAKE_PROC_CMDLINE_FILE}
${SUSFS_BIN} set_cmdline_or_bootconfig ${FAKE_PROC_CMDLINE_FILE}
EOF

#### Enable sus_su (Deprecated, do NOT use it) ####
#cat <<EOF >/dev/null
#enable_sus_su_mode_1(){
#	## Here we manually create an system overlay an copy the sus_su and sus_su_drv_path to ${MODDIR}/system/bin after sus_su is enabled,
#	## as ksu overlay script is executed after all post-fs-data.sh scripts are finished
#
#	rm -rf ${MODDIR}/system 2>/dev/null
#	# Enable sus_su or abort the function if sus_su is not supported #
#	if ! ${SUSFS_BIN} sus_su 1; then
#		return
#	fi
#	mkdir -p ${MODDIR}/system/bin 2>/dev/null
#	# Copy the new generated sus_su_drv_path and 'sus_su' to /system/bin/ and rename 'sus_su' to 'su' #
#	cp -f /data/adb/ksu/bin/sus_su ${MODDIR}/system/bin/su
#	cp -f /data/adb/ksu/bin/sus_su_drv_path ${MODDIR}/system/bin/sus_su_drv_path
#}
## NOTE: mode 1 has to be run in post-fs-data.sh stage as it needs ksu default overlay mount scheme to mount the su overlay #
## uncomment it below to enable sus_su with mode 1 #
##enable_sus_su_mode_1
#EOF

#### Hiding the exposed /proc interface of ext4 loop and jdb2 when mounting modules.img using sus_path ####
cat <<EOF >/dev/null
for device in $(ls -Ld /proc/fs/jbd2/loop*8 | sed 's|/proc/fs/jbd2/||; s|-8||'); do
	${SUSFS_BIN} add_sus_path /proc/fs/jbd2/${device}-8
	${SUSFS_BIN} add_sus_path /proc/fs/ext4/${device}
done
EOF

#### Umount for all zygote spawned services ####
cat <<EOF >/dev/null
# - set to 1 to enable umount for all zygote spawned services, but be reminded that
#   it may break some modules that ovsrlay framework files / overlay apks. Boot into
#   KSU rescue mode if you encounter bootloop here.
ksu_susfs umount_for_zygote_iso_service 1
EOF
