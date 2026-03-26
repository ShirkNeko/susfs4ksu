#!/system/bin/sh
PATH=/data/adb/ksu/bin:$PATH

MODDIR=/data/adb/modules/susfs4ksu

SUSFS_BIN=/data/adb/ksu/bin/ksu_susfs

source ${MODDIR}/utils.sh

## Important Notes:
## - The following command can be run at other stages like service.sh, boot-completed.sh etc..,
## - This module is just an demo showing how to use ksu_susfs tool to commuicate with kernel
##

#### Spoof the stat of file/directory dynamically, effective only for processes that are marked umounted with uid >= 10000 ####
## Important Note:
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

#### Spoof the stat of file/directory statically, effective only for processes that are marked umounted with uid >= 10000 ####
## Important Note:
##  - It is suggested to use statically if you don't need to mount anything but simply change the stat of a target path
cat <<EOF >/dev/null
Usage: ksu_susfs add_sus_kstat_statically </path/of/file_or_directory> \
                        <ino> <dev> <nlink> <size> <atime> <atime_nsec> <mtime> <mtime_nsec> <ctime> <ctime_nsec> \
                        <blocks> <blksize>
${SUSFS_BIN} add_sus_kstat_statically '/system/framework/services.jar' 'default' 'default' 'default' 'default' '1230768000' '0' '1230768000' '0' '1230768000' '0' 'default' 'default'
EOF

#### Spoof the uname, effective for all processes ####
# you can get your uname args by running 'uname {-r|-v}' on your stock ROM #
# pass 'default' to tell susfs to use the default value by uname #
cat <<EOF >/dev/null
${SUSFS_BIN} set_uname '5.15.137-android14-11-gb572b1fac135-ab11919372' '#1 SMP PREEMPT Mon Jun 3 16:35:10 UTC 2024'
EOF

#### Redirect path, effective for processes with uid < 2000 ####
# redirect hosts file to other hosts file somewhere else #
cat <<EOF >/dev/null
# First make sure you setup proper permission and selinux for your redirected file #
susfs_clone_perm '/data/local/tmp/my_hosts' '/system/etc/hosts'
# And then add them via 'add_path_redirect'
${SUSFS_BIN} add_path_redirect '/system/etc/hosts' '/data/local/tmp/my_hosts'
EOF

#### Spoof /proc/cmdline or /proc/bootconfig, effective for all processes ####
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
${SUSFS_BIN} set_cmdline_or_bootconfig ${FAKE_BOOTCONFIG}
EOF

# /proc/cmdline #
cat <<EOF >/dev/null
FAKE_PROC_CMDLINE_FILE=${MODDIR}/fake_proc_cmdline.txt
cat /proc/cmdline > ${FAKE_PROC_CMDLINE_FILE}
sed -i 's/androidboot.verifiedbootstate=orange/androidboot.verifiedbootstate=green/g' ${FAKE_PROC_CMDLINE_FILE}
sed -i 's/androidboot.vbmeta.device_state=unlocked/androidboot.vbmeta.device_state=locked/g' ${FAKE_PROC_CMDLINE_FILE}
${SUSFS_BIN} set_cmdline_or_bootconfig ${FAKE_PROC_CMDLINE_FILE}
EOF

#### Hiding the exposed /proc interface of ext4 loop and jdb2 when mounting ext4 img using sus_path ####
cat <<EOF >/dev/null
## Hide all sus ext4 loops and jbd2 journals if they are still mounted and with jdb2 journal enabled ##
for device in $(ls -Ld /proc/fs/jbd2/loop*8 | sed 's|/proc/fs/jbd2/||; s|-8||'); do
	${SUSFS_BIN} add_sus_path /proc/fs/jbd2/${device}-8
	${SUSFS_BIN} add_sus_path /proc/fs/ext4/${device}
done
## Also we need to spoof the nlink of /proc/fs/jbd2 to 2 ##
${SUSFS_BIN} add_sus_kstat_statically '/proc/fs/jbd2' 'default' 'default' '2' 'default' 'default' 'default' 'default' 'default' 'default' 'default' 'default' 'default'
EOF

#### Enable avc log spoofing to bypass 'su' domain detection via /proc/<pid> enumeration, effective for all processes ####
cat <<EOF >/dev/null
ksu_susfs enable_avc_log_spoofing 1
## disable it when users want to do some debugging with the permission issue or selinux issue ##
#ksu_susfs enable_avc_log_spoofing 0
EOF

#### Hide all sus mounts for NON-SU processes in this stage just to prevent zygote from caching them in memory ####
## This should be mainly applied if you have ReZygisk enabled but without TreatWheel module ##
## Or it is up to you to keep it enabled since su process can still see the mounts ##
cat <<EOF >/dev/null
ksu_susfs hide_sus_mnts_for_non_su_procs 1
EOF


