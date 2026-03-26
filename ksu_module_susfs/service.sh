#!/system/bin/sh
PATH=/data/adb/ksu/bin:$PATH

MODDIR=/data/adb/modules/susfs4ksu

SUSFS_BIN=/data/adb/ksu/bin/ksu_susfs

source ${MODDIR}/utils.sh

## Delete some prop names for newer pixel device ##
cat <<EOF >/dev/null
resetprop --delete "ro.boot.verifiedbooterror"
resetprop --delete "ro.boot.verifyerrorpart"
resetprop --delete "crashrecovery.rescue_boot_count"
## Remember to compat the hole after prop deletion (require updated resetprop)##
resetprop -c
EOF

## Undo hiding sus mounts for all non-su processes ##
cat <<EOF >/dev/null
ksu_susfs hide_sus_mnts_for_non_su_procs 0
EOF

## Disable susfs kernel log ##
#${SUSFS_BIN} enable_log 0

