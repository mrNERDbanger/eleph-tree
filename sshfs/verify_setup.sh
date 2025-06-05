#!/bin/bash
#
# Verify SSHFS FUSE3 setup
#

echo "🐘 SSHFS FUSE3 Setup Verification"
echo "================================="
echo

# Check FUSE3 compatibility layer
echo -n "FUSE3 compatibility layer: "
if [ -f /usr/local/lib/libfuse3_compat.dylib ] && [ -f /usr/local/include/fuse3.h ]; then
    echo "✓ Installed"
else
    echo "✗ Missing"
fi

# Check macFUSE
echo -n "macFUSE: "
if [ -d /usr/local/include/osxfuse ] || [ -d /Library/Frameworks/macFUSE.framework ]; then
    echo "✓ Installed"
else
    echo "✗ Missing"
fi

# Check libssh2
echo -n "libssh2: "
if pkg-config --exists libssh2 2>/dev/null; then
    echo "✓ Found ($(pkg-config --modversion libssh2))"
else
    echo "✗ Missing"
fi

# Check built binaries
echo
echo "Built SSHFS binaries:"
for binary in sshfs_v3 sshfs_fuse3; do
    echo -n "  $binary: "
    if [ -x "./$binary" ]; then
        echo "✓ Built"
    else
        echo "✗ Missing"
    fi
done

echo
echo "System info:"
echo "  Architecture: $(uname -m)"
echo "  macOS version: $(sw_vers -productVersion)"
echo

# Check if we can link against FUSE3
echo "Testing FUSE3 library linkage:"
cat > /tmp/test_fuse3.c << 'EOF'
#include <fuse3.h>
#include <stdio.h>
int main() {
    printf("FUSE3 library test: OK\n");
    return 0;
}
EOF

if clang -o /tmp/test_fuse3 /tmp/test_fuse3.c -lfuse3_compat -L/usr/local/lib -I/usr/local/include 2>/dev/null; then
    echo "✓ FUSE3 library linkage works"
    /tmp/test_fuse3
    rm -f /tmp/test_fuse3
else
    echo "✗ FUSE3 library linkage failed"
fi

rm -f /tmp/test_fuse3.c

echo
echo "Ready to test with Flywheel SSH!"
echo "Run: ./test_flywheel.sh"