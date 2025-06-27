#!/system/bin/sh
PATH=/data/adb/ksu/bin:$PATH

MODDIR=/data/adb/modules/susfs4ksu

SUSFS_BIN=/data/adb/ksu/bin/ksu_susfs

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
