#!/system/bin/sh

PKG="com.example.dummy3"
LOG_TAG="liho"

log() {
    echo "$LOG_TAG $1" > /dev/kmsg
}

cleanup() {
    for dir in /data/app/*/${PKG}*/oat; do
        if [ -d "$dir" ]; then
            rm -rf "$dir"
            log "Deleted $dir"
        fi
    done
    pm compile --reset "$PKG" >/dev/null 2>&1 && log "Reset compilation $PKG"
    cmd package compile -m verify -f "$PKG" >/dev/null 2>&1 && log "Set verify mode $PKG"
}

until ls /data/app/*/${PKG}* >/dev/null 2>&1; do
    sleep 2
done

while true; do
    cleanup
    sleep 3
done