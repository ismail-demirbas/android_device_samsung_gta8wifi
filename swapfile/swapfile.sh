#!/system/bin/sh
D=/data/unencrypted
F=$D/swapfile
S=$D/swapfile.status
FLAG=$D/.swapfile
SZ=4096

if [ "$1" = off ]; then
  swapoff $F 2>/dev/null
  rm -f $F
  echo "off" > $S
  exit 0
fi

if grep -q "^$F " /proc/swaps; then
  echo "already active" > $S
  exit 0
fi

if [ ! -f $F ] || [ "$(stat -c %s $F)" != "$((SZ*1048576))" ]; then
  rm -f $F $F.tmp
  AV=$(df -k $D | tail -n 1 | tr -s ' ' | cut -d' ' -f4)
  if [ -z "$AV" ] || [ "$AV" -lt $((SZ*1024+524288)) ]; then
    echo "no space (avail ${AV}kB)" > $S
    exit 1
  fi
  dd if=/dev/zero of=$F.tmp bs=1M count=$SZ 2>/dev/null || { rm -f $F.tmp; echo "dd failed" > $S; exit 1; }
  sync
  chmod 600 $F.tmp
  mkswap $F.tmp >/dev/null 2>&1 || { rm -f $F.tmp; echo "mkswap failed" > $S; exit 1; }
  mv $F.tmp $F || { rm -f $F.tmp; echo "mv failed" > $S; exit 1; }
fi

i=0
while [ $i -lt 40 ] && [ "$(grep -m1 SwapTotal /proc/meminfo | tr -s ' ' | cut -d' ' -f2)" = 0 ]; do
  sleep 0.5
  i=$((i+1))
done
if swapon $F; then
  echo "active $SZ MB" > $S
else
  echo "swapon failed rc=$?" > $S
  exit 1
fi
