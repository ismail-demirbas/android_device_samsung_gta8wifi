#!/bin/bash
RD=$(cd "$(dirname "$0")" && pwd)
cd "$RD" || exit 1
dev=$(basename "$RD")
OUT=$(cd "$RD/../../.." && pwd)/out/target/product/$dev
BR=$(git branch --show-current)
GH=$(git remote get-url origin | sed -E 's#.*github.com[:/]##; s#\.git$##')
zip=$(ls -t "$OUT"/lineage-*-"$dev".zip 2>/dev/null | head -1)
[ -n "$zip" ] || { echo "ERROR: no zip found in $OUT"; exit 1; }
f=$(basename "$zip"); tag=${f%.zip}; sz=$(stat -c%s "$zip")
python3 - "$zip" "$dev" "$GH" "$tag" "$OUT/system/build.prop" <<'PY' || exit 1
import sys,json,os,re,hashlib
zip,dev,gh,tag,bp=sys.argv[1:6]
f=os.path.basename(zip)
m=re.match(r'lineage-(\d+\.\d+)-(\d{8})-([A-Za-z]+)-',f)
if not m: print('ERROR: unexpected file name',f); sys.exit(1)
ver,day,typ=m.groups()
dt=0
if os.path.exists(bp):
    for l in open(bp):
        if l.startswith('ro.build.date.utc='): dt=int(l.split('=')[1])
if not dt: dt=int(os.path.getmtime(zip))
h=hashlib.sha256()
with open(zip,'rb') as z:
    for b in iter(lambda:z.read(1<<20),b''): h.update(b)
sha=h.hexdigest(); size=os.path.getsize(zip)
e={'datetime':dt,'files':[{'filename':f,'sha256':sha,'size':size,'url':f'https://github.com/{gh}/releases/download/{tag}/{f}'}],'type':typ.upper(),'version':ver}
p=f'{dev}.json'
l=json.load(open(p)) if os.path.exists(p) else []
l=[x for x in l if x['files'][0]['filename']!=f]+[e]
l.sort(key=lambda x:-x['datetime'])
json.dump(l,open(p,'w'),indent=2)
d=f'{day[:4]}-{day[4:6]}-{day[6:]}'
title=f'LineageOS {ver} ({d}) - {dev}'
notes=f'## {title}\n\n- Device: {dev}\n- Version: LineageOS {ver}\n- Build type: {typ.upper()}\n- Build date: {d}\n- File: `{f}`\n- Size: {size//1048576} MB\n- SHA256: `{sha}`\n'
open('/tmp/ota_title.txt','w').write(title)
open('/tmp/ota_notes.md','w').write(notes)
print('OK',p)
PY
title=$(cat /tmp/ota_title.txt)
echo
echo "zip   : $f ($((sz/1048576)) MB)"
echo "tag   : $tag"
echo "title : $title"
echo "repo  : $GH ($BR)"
[ "$sz" -lt 2147483648 ] || echo "WARNING: over 2 GiB, GitHub may reject this file"
read -r -p "Extra release notes (optional, Enter to skip): " extra
[ -z "$extra" ] || printf '\n### Notes\n%s\n' "$extra" >> /tmp/ota_notes.md
echo "----- release notes -----"; cat /tmp/ota_notes.md; echo "-------------------------"
read -r -p "Upload to GitHub Releases and push $dev.json? [y/N] " a
if [ "$a" = "y" ] || [ "$a" = "Y" ]; then
  git fetch -q origin "$BR"
  ahead=$(git rev-list --count "origin/$BR..HEAD")
  if [ "$ahead" -gt 0 ]; then
    echo "ERROR: $ahead unpushed local commit(s) on $BR; push the device tree first. Nothing uploaded."
    exit 1
  fi
  if gh release view "$tag" --repo "$GH" >/dev/null 2>&1; then
    gh release upload "$tag" "$zip" --repo "$GH" --clobber || { echo "ERROR: upload failed"; exit 1; }
  else
    gh release create "$tag" "$zip" --repo "$GH" --target "$BR" --title "$title" --notes-file /tmp/ota_notes.md || { echo "ERROR: release failed"; exit 1; }
  fi
  git add "$dev.json"
  git commit -qm "$dev.json updated" -- "$dev.json"
  git push origin "$BR" && echo OK
else
  echo "Not uploaded, not pushed. $dev.json is ready locally."
  echo "Manual: gh release create $tag $zip --repo $GH --target $BR --title \"$title\" --notes-file /tmp/ota_notes.md"
  echo "Then:   git add $dev.json && git commit -m \"$dev.json updated\" -- $dev.json && git push origin $BR"
fi
