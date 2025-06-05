#!/bin/bash
#
# FUSE3 Compatibility Layer Installation Script
# This script installs the FUSE3 compatibility layer for macFUSE
#

set -e

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Installation directories
PREFIX="${PREFIX:-/usr/local}"
LIBDIR="${PREFIX}/lib"
INCLUDEDIR="${PREFIX}/include"

# Print colored message
print_msg() {
    local color=$1
    shift
    echo -e "${color}$@${NC}"
}

# Check if running as root when needed
check_root() {
    if [[ $EUID -ne 0 ]] && [[ "$PREFIX" == "/usr/local" ]]; then
        print_msg $RED "Error: Installation to /usr/local requires root privileges."
        print_msg $YELLOW "Run with sudo or specify a different PREFIX:"
        print_msg $YELLOW "  PREFIX=~/local ./install.sh"
        exit 1
    fi
}

# Check for macFUSE installation
check_macfuse() {
    print_msg $YELLOW "Checking for macFUSE installation..."
    
    if [[ ! -d "/usr/local/include/fuse" ]] && [[ ! -d "/Library/Frameworks/macFUSE.framework" ]]; then
        print_msg $RED "Error: macFUSE is not installed."
        print_msg $YELLOW "Please install macFUSE from: https://osxfuse.github.io/"
        exit 1
    fi
    
    print_msg $GREEN "✓ macFUSE found"
}

# Build the library
build_library() {
    print_msg $YELLOW "Building FUSE3 compatibility layer..."
    
    if ! make clean && make; then
        print_msg $RED "Error: Build failed"
        exit 1
    fi
    
    print_msg $GREEN "✓ Build successful"
}

# Install files
install_files() {
    print_msg $YELLOW "Installing files..."
    
    # Create directories if they don't exist
    mkdir -p "${LIBDIR}"
    mkdir -p "${INCLUDEDIR}"
    
    # Install library
    cp -f libfuse3_compat.dylib "${LIBDIR}/"
    ln -sf libfuse3_compat.dylib "${LIBDIR}/libfuse3_compat.1.dylib"
    
    # Install header
    cp -f fuse3.h "${INCLUDEDIR}/"
    
    # Install pkg-config file if pkg-config directory exists
    if [[ -d "${PREFIX}/lib/pkgconfig" ]]; then
        cat > "${PREFIX}/lib/pkgconfig/fuse3.pc" << EOF
prefix=${PREFIX}
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include

Name: fuse3
Description: FUSE3 compatibility layer for macFUSE
Version: 3.0.0
Libs: -L\${libdir} -lfuse3_compat
Cflags: -I\${includedir}
EOF
    fi
    
    print_msg $GREEN "✓ Files installed to ${PREFIX}"
}

# Update library cache (if applicable on macOS)
update_cache() {
    print_msg $YELLOW "Updating library cache..."
    
    # On macOS, we might need to update dyld cache
    if command -v update_dyld_shared_cache &> /dev/null; then
        update_dyld_shared_cache || true
    fi
    
    print_msg $GREEN "✓ Library cache updated"
}

# Main installation process
main() {
    print_msg $GREEN "FUSE3 Compatibility Layer Installer"
    print_msg $GREEN "==================================="
    echo
    
    # Check prerequisites
    check_macfuse
    check_root
    
    # Build and install
    build_library
    install_files
    update_cache
    
    echo
    print_msg $GREEN "✓ Installation complete!"
    print_msg $YELLOW ""
    print_msg $YELLOW "To use the FUSE3 compatibility layer:"
    print_msg $YELLOW "  - Link with: -lfuse3_compat"
    print_msg $YELLOW "  - Include: #include <fuse3.h>"
    print_msg $YELLOW ""
    print_msg $YELLOW "Example compilation:"
    print_msg $YELLOW "  gcc -o myfs myfs.c -lfuse3_compat -L${LIBDIR}"
}

# Run main installation
main "$@"