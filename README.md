# 🐘 eleph-tree

**FUSE API v3 compatibility layer for macOS**

*By Elephunkie*

eleph-tree provides FUSE API version 3 support for macFUSE, enabling modern FUSE applications to run seamlessly on macOS. The name combines "elephant" (our company mascot) with "tree" (representing both the "three" in v3 and file system tree structures).

## Project Structure

```
eleph-tree/
├── macfuse/                    # macFUSE repository with v3 support
│   ├── Library-3/              # FUSE v3 compatibility layer
│   │   ├── fuse3.h            # FUSE v3 API header
│   │   ├── fuse3_compat.c     # Compatibility implementation
│   │   └── Makefile           # Build system for v3 library
│   ├── Library-2/              # Original FUSE v2 library (empty)
│   └── Framework/              # macFUSE framework (empty)
├── sshfs/                      # SSHFS with FUSE v3 support
│   ├── sshfs_v3.c             # SSHFS implementation using FUSE v3 API
│   ├── Makefile_v3            # Build system for SSHFS v3
│   └── [original sshfs files] # Original SSHFS files
└── README.md                   # This file
```

## FUSE v3 Compatibility Layer

The FUSE v3 compatibility layer (`macfuse/Library-3/`) provides:

- **fuse3.h**: Complete FUSE v3 API header definitions
- **fuse3_compat.c**: Implementation that translates FUSE v3 calls to macFUSE v2
- **Makefile**: Build system to create libfuse3_compat.dylib

### Key Features

- Full FUSE v3 API compatibility
- Seamless integration with existing macFUSE infrastructure
- Support for modern FUSE features like:
  - Enhanced file info structures
  - Improved buffer management
  - Extended attributes
  - Better error handling

## Building

### 1. Build the FUSE v3 Compatibility Library

```bash
cd macfuse/Library-3
make
```

This creates `libfuse3_compat.dylib` which provides FUSE v3 API on top of macFUSE v2.

### 2. Build SSHFS with FUSE v3 Support

```bash
cd sshfs
make -f Makefile_v3
```

This creates `sshfs_v3` executable that uses the FUSE v3 API.

## Usage

### SSHFS v3 Example

```bash
# Mount a remote filesystem using SSHFS v3
./sshfs_v3 user@hostname:/remote/path /local/mountpoint

# Unmount
umount /local/mountpoint
```

## API Differences: FUSE v2 vs v3

| Feature | FUSE v2 | FUSE v3 |
|---------|---------|---------|
| API Version | 26-29 | 30+ |
| File Info | `struct fuse_file_info` | `struct fuse3_file_info` |
| Operations | `struct fuse_operations` | `struct fuse3_operations` |
| Readdir | Simple callback | Enhanced with flags |
| Buffer I/O | Basic | Vectorized buffers |
| Error Handling | errno codes | Enhanced error reporting |

## Installation

```bash
# Install the compatibility library
cd macfuse/Library-3
sudo make install

# Install SSHFS v3
cd sshfs
sudo make -f Makefile_v3 install
```

## Dependencies

- macFUSE (installed via Homebrew or official installer)
- libssh2 (for SSHFS functionality)
- fuse-t (alternative FUSE implementation)

## Implementation Notes

The compatibility layer works by:

1. **API Translation**: Converting FUSE v3 function calls to v2 equivalents
2. **Structure Mapping**: Translating v3 data structures to v2 format
3. **Feature Emulation**: Implementing v3 features using v2 primitives
4. **Error Handling**: Maintaining v3 error semantics

## Limitations

- Some advanced FUSE v3 features may have limited support
- Performance overhead due to API translation layer
- Requires existing macFUSE v2 installation

## Contributing

This is a compatibility layer implementation. Key areas for improvement:

- Enhanced error handling
- Performance optimizations
- Additional FUSE v3 feature support
- Better integration with macOS-specific features

## License

This project follows the same licensing as the underlying macFUSE and SSHFS projects.