#!/usr/bin/env bash
# Bundles server_daemon and its non-system shared libraries into a
# relocatable tarball that can be extracted on another Linux machine.
#
# usage: deploy/package.sh [build-bin-dir] [output-dir]
#   build-bin-dir  folder containing server_daemon and libserver_library.so (default: build/bin)
#   output-dir     where the tarball is written (default: dist)
#
# Requires: patchelf
# Target needs: glibc >= the version printed at the end, and libcurl
set -euo pipefail

BIN=${1:-build/bin}
DIST=${2:-dist}
OUT="$DIST/qms"
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

if [[ ! -x "$BIN/server_daemon" ]]; then
    echo "server_daemon not found in $BIN" >&2
    exit 1
fi
command -v patchelf >/dev/null || { echo "patchelf is required" >&2; exit 1; }

rm -rf "$OUT"
mkdir -p "$OUT"

cp "$BIN/server_daemon" "$OUT/"
cp -P "$BIN"/libserver_library.so* "$OUT/"

# Copy shared-library dependencies. Base-system libraries (glibc, libstdc++)
# and libcurl with its TLS stack are left to the target system, so curl uses
# the target distro's CA certificate locations.
EXCLUDE='/(ld-linux|libc|libm|libpthread|libdl|librt|libstdc\+\+|libgcc_s|libcurl|libssl|libcrypto|libz|libnghttp2|libnghttp3|libngtcp2|libidn2|libpsl|libssh|libssh2|librtmp|libbrotlidec|libbrotlicommon|libzstd|libgssapi_krb5|libkrb5|libkrb5support|libk5crypto|libcom_err|libldap|liblber|libsasl2|libunistring|libkeyutils|libresolv|libgnutls|libnettle|libhogweed|libgmp|libtasn1|libp11-kit|libffi)[.-]'

ldd "$BIN/server_daemon" "$BIN/libserver_library.so" \
    | awk '/=> \//{print $3}' | sort -u \
    | grep -Ev "$EXCLUDE" \
    | while read -r lib; do
        case "$(basename "$lib")" in
            libserver_library.so*) ;;  # already copied
            *) cp -L "$lib" "$OUT/" ;;
        esac
    done

# Every binary looks for its libraries in its own folder
patchelf --set-rpath '$ORIGIN' "$OUT/server_daemon"
for f in "$OUT"/*.so*; do
    [[ -L "$f" ]] || patchelf --set-rpath '$ORIGIN' "$f"
done

cp "$SCRIPT_DIR/qms.service" "$OUT/"
if [[ -f "$BIN/config.json" ]]; then
    cp "$BIN/config.json" "$OUT/config.json.example"
fi

ARCH=$(uname -m)
tar czf "$DIST/qms-linux-$ARCH.tar.gz" -C "$DIST" qms

echo "Bundled files:"
ls -1 "$OUT"
echo
echo "Minimum glibc required on target:"
objdump -T "$OUT"/server_daemon "$OUT"/*.so* 2>/dev/null \
    | grep -o 'GLIBC_[0-9.]*' | sort -Vu | tail -1
echo
echo "Package: $DIST/qms-linux-$ARCH.tar.gz"
