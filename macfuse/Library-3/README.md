# FUSE3 Compatibility Layer for macFUSE

This library provides a FUSE v3 API compatibility layer for macFUSE, allowing FUSE v3 applications to run on macOS without modification.

## Overview

macFUSE implements the FUSE v2 API, but many modern FUSE applications are written for FUSE v3. This compatibility layer translates FUSE v3 function calls and data structures to their FUSE v2 equivalents, enabling seamless integration.

## Features

### Implemented Operations
- **File Operations**: getattr, readlink, mknod, mkdir, unlink, rmdir
- **I/O Operations**: open, read, write, release  
- **Core Functions**: fuse3_new, fuse3_loop, fuse3_destroy
- **Utilities**: Command line parsing, file info structure conversion

### Current Limitations
- No support for extended attributes (xattr operations)
- Limited directory operations (no readdir)
- Missing advanced features (polling, locking, fallocate)
- Basic command line parsing only
- Mount point handling needs improvement

## Building

```bash
make                    # Build the library and example
make install           # Install to /usr/local (requires sudo)
make test              # Build and run the example
make clean             # Clean build artifacts
```

## Usage

### Linking Your Application

Replace `-lfuse3` with `-lfuse3_compat` in your build:

```bash
gcc -o myfs myfs.c -lfuse3_compat -L/usr/local/lib
```

### Example Application

See `hello_fuse3.c` for a complete example of a read-only filesystem using the FUSE v3 API.

```c
#include "fuse3.h"

static int hello_getattr(const char *path, struct stat *stbuf,
                        struct fuse3_file_info *fi) {
    // Implementation
}

static struct fuse3_operations hello_oper = {
    .getattr = hello_getattr,
    .readdir = hello_readdir,
    .open    = hello_open,
    .read    = hello_read,
};

int main(int argc, char *argv[]) {
    return fuse3_main(argc, argv, &hello_oper, NULL);
}
```

## Installation

1. Ensure macFUSE is installed on your system
2. Build the compatibility layer: `make`
3. Install system-wide: `sudo make install`
4. Update your application's build to link against `libfuse3_compat`

## Architecture

The compatibility layer works by:
1. Providing FUSE v3 API function signatures
2. Converting FUSE v3 structures to v2 equivalents
3. Wrapping operation callbacks with conversion functions
4. Delegating actual filesystem operations to macFUSE (FUSE v2)

## Future Improvements

- [ ] Complete implementation of all FUSE v3 operations
- [ ] Proper mount point handling
- [ ] Full command line option parsing
- [ ] Support for FUSE v3 specific features
- [ ] Better error handling and logging
- [ ] Performance optimizations

## License

This project is licensed under the same terms as macFUSE.

## Contributing

Contributions are welcome! Key areas needing work:
- Implementing missing operations
- Improving mount/unmount handling  
- Adding comprehensive test suite
- Documentation improvements