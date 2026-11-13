#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

import os
import re

from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)

namespace_imports = [
    'device/samsung/gta8wifi',
]


class GTA8FixupsModule(ExtractUtilsModule):
    def write_makefiles(self, legacy, extract_factory):
        super().write_makefiles(legacy, extract_factory)

        path = os.path.join(self.vendor_path, "Android.bp")
        text = open(path, encoding="utf-8").read()

        fixes = (
            ("vendor.samsung.hardware.hyper-service",
             '"libhyper"', '"libhyper_vnd"'),
            ("vendor.sprd.hardware.trusty@1.0-service",
             '"libclang_rt.ubsan_standalone-aarch64-android"',
             '"libclang_rt.ubsan_standalone"'),
            ("vendor.sprd.hardware.trusty@1.0-impl",
             '"libclang_rt.ubsan_standalone-aarch64-android"',
             '"libclang_rt.ubsan_standalone"'),
            ("android.hardware.drm@1.3-service.clearkey",
             '"libprotobuf-cpp-lite-3.9.1"',
             '"libprotobuf-cpp-lite-3.9.1-vendorcompat"'),
        )

        for module, old, new in fixes:
            marker = f'name: "{module}"'
            if text.count(marker) != 1:
                raise RuntimeError(f"Module not found uniquely: {module}")

            pos = text.index(marker) + len(marker)
            next_module = re.search(
                r"(?m)^[A-Za-z_][A-Za-z0-9_]*\s*\{", text[pos:]
            )
            end = pos + next_module.start() if next_module else len(text)
            block = text[pos:end]

            lists = list(re.finditer(
                r"shared_libs:\s*\[(.*?)\]", block, re.S
            ))

            count = sum(m.group(1).count(old) for m in lists)
            if count == 0:
                if any(new in m.group(1) for m in lists):
                    print(f"ALREADY OK: {module}")
                    continue
                raise RuntimeError(f"Dependency not found: {module}: {old}")

            for match in reversed(lists):
                deps = match.group(1)
                if old in deps:
                    deps = deps.replace(old, new)
                    block = (
                        block[:match.start(1)] + deps
                        + block[match.end(1):]
                    )

            text = text[:pos] + block + text[end:]
            print(f"FIXED: {module}")
            print(f"       {old.strip(chr(34))} -> {new.strip(chr(34))}")
            print(f"       Occurrences: {count}")

        temp = path + ".tmp-dependency-fixups"
        with open(temp, "w", encoding="utf-8") as f:
            f.write(text)
        os.replace(temp, path)

        print(f"GENERATED FILE FIXED: {path}")
module = GTA8FixupsModule(
    'gta8wifi',
    'samsung',
    namespace_imports=namespace_imports,
)

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
