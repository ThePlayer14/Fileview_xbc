# Fileview-xbc

# What does this tool do?
This tool takes in a file created by the Xbox 360 File Compression Tool, and using the XCompress library, it prints out several attributes:
* File header information: Identifier, Version, Content flags
* Codec Parameters: Flags, Window Size, Compression Partition Size
* File Sizes: Uncompressed/Compressed size, Compress Ratio, Uncompressed Block Size, Compressed Block Max Size
* File stats: Total File Size, File name

# Supported formats
* `LZXNATIVE` (0xFF512EE) and `LZXTDECODE` (0xFF512ED) format

# Additional features:
* Display data in little-endian (default) and Big-endian (via `--swap` option)
Note that `0xFF512EE` is the Big Endian identifier, so by default you'll see `0xEE12F50F` as the identifier
* Display information in bytes

# Note
* This tool was made using Qwen3-Coder.
