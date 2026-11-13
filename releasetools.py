import common


def FullOTA_InstallEnd(info):
    data = info.input_zip.read("RADIO/vbmeta.img")
    common.ZipWriteStr(info.output_zip, "vbmeta.img", data)
    info.script.AppendExtra('package_extract_file("vbmeta.img", "/dev/block/by-name/vbmeta");')
