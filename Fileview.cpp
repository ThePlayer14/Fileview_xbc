#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <windows.h>
#include "xcompress.h"

using namespace std;

// Function to read file into memory
vector<BYTE> ReadFile(const string& filename) {
    ifstream file(filename, ios::binary | ios::ate);
    if (!file.is_open()) {
        throw runtime_error("Could not open file: " + filename);
    }
    
    streamsize size = file.tellg();
    file.seekg(0, ios::beg);
    
    vector<BYTE> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw runtime_error("Could not read file: " + filename);
    }
    
    return buffer;
}

// Function to swap byte order for 32-bit values
DWORD SwapBytes32(DWORD value) {
    return ((value & 0x000000FF) << 24) |
           ((value & 0x0000FF00) << 8)  |
           ((value & 0x00FF0000) >> 8)  |
           ((value & 0xFF000000) >> 24);
}

// Function to swap byte order for 16-bit values
WORD SwapBytes16(WORD value) {
    return ((value & 0x00FF) << 8) | ((value & 0xFF00) >> 8);
}

// Function to display compression header information (normal byte order)
void DisplayHeaderInfo(const XCOMPRESS_FILE_HEADER_LZXNATIVE& header) {
    cout << "=== File Header Information ===" << endl;
    cout << "Identifier: 0x" << hex << uppercase << header.Common.Identifier << dec << endl;
    cout << "Version: " << XCOMPRESS_GET_FILE_VERSION_MAJOR(header.Common.Version) 
         << "." << XCOMPRESS_GET_FILE_VERSION_MINOR(header.Common.Version) << endl;
    cout << "Context Flags: 0x" << hex << header.ContextFlags << dec << endl;
    
    // Display codec parameters
    cout << "\n=== Codec Parameters ===" << endl;
    cout << "Flags: 0x" << hex << header.CodecParams.Flags << dec << endl;
    cout << "Window Size: " << header.CodecParams.WindowSize << " bytes" << endl;
    cout << "Compression Partition Size: " << header.CodecParams.CompressionPartitionSize << " bytes" << endl;
    
    // Display size information
    cout << "\n=== File Sizes ===" << endl;
    DWORD64 uncompressedSize = ((DWORD64)header.UncompressedSizeHigh << 32) | header.UncompressedSizeLow;
    DWORD64 compressedSize = ((DWORD64)header.CompressedSizeHigh << 32) | header.CompressedSizeLow;
    
    cout << "Uncompressed Size: " << uncompressedSize << " bytes" << endl;
    cout << "Compressed Size: " << compressedSize << " bytes" << endl;
    cout << "Compression Ratio: " 
         << fixed << setprecision(2) 
         << (double)uncompressedSize / (double)compressedSize << ":1" << endl;
    
    cout << "Uncompressed Block Size: " << header.UncompressedBlockSize << " bytes" << endl;
    cout << "Compressed Block Max Size: " << header.CompressedBlockSizeMax << " bytes" << endl;
}

// Function to display compression header information (swapped byte order)
void DisplayHeaderInfoSwapped(const XCOMPRESS_FILE_HEADER_LZXNATIVE& header) {
    cout << "=== File Header Information (Big-endian Order) ===" << endl;
    
    // Swap the identifier
    DWORD swappedIdentifier = SwapBytes32(header.Common.Identifier);
    cout << "Identifier: 0x" << hex << uppercase << swappedIdentifier << dec << endl;
    
    // Swap version components
    WORD swappedVersion = SwapBytes16(header.Common.Version);
    cout << "Version: " << XCOMPRESS_GET_FILE_VERSION_MAJOR(swappedVersion) 
         << "." << XCOMPRESS_GET_FILE_VERSION_MINOR(swappedVersion) << endl;
    
    // Swap context flags
    DWORD swappedContextFlags = SwapBytes32(header.ContextFlags);
    cout << "Context Flags: 0x" << hex << swappedContextFlags << dec << endl;
    
    // Display codec parameters (swapped)
    cout << "\n=== Codec Parameters (Big-endian Order) ===" << endl;
    DWORD swappedFlags = SwapBytes32(header.CodecParams.Flags);
    cout << "Flags: 0x" << hex << swappedFlags << dec << endl;
    
    DWORD swappedWindowSize = SwapBytes32(header.CodecParams.WindowSize);
    cout << "Window Size: " << swappedWindowSize << " bytes" << endl;
    
    DWORD swappedPartitionSize = SwapBytes32(header.CodecParams.CompressionPartitionSize);
    cout << "Compression Partition Size: " << swappedPartitionSize << " bytes" << endl;
    
    // Display size information (swapped)
    cout << "\n=== File Sizes (Big-endian Order) ===" << endl;
    DWORD64 swappedUncompressedSize = ((DWORD64)SwapBytes32(header.UncompressedSizeHigh) << 32) | SwapBytes32(header.UncompressedSizeLow);
    DWORD64 swappedCompressedSize = ((DWORD64)SwapBytes32(header.CompressedSizeHigh) << 32) | SwapBytes32(header.CompressedSizeLow);
    
    cout << "Uncompressed Size: " << swappedUncompressedSize << " bytes" << endl;
    cout << "Compressed Size: " << swappedCompressedSize << " bytes" << endl;
    
    // Calculate ratio with swapped values
    if (swappedCompressedSize > 0) {
        double ratio = (double)swappedUncompressedSize / (double)swappedCompressedSize;
        cout << "Compression Ratio: " << fixed << setprecision(2) << ratio << ":1" << endl;
    }
    
    DWORD swappedUncompressedBlockSize = SwapBytes32(header.UncompressedBlockSize);
    DWORD swappedCompressedBlockSize = SwapBytes32(header.CompressedBlockSizeMax);
    cout << "Uncompressed Block Size: " << swappedUncompressedBlockSize << " bytes" << endl;
    cout << "Compressed Block Max Size: " << swappedCompressedBlockSize << " bytes" << endl;
}

// Function to analyze file structure
void AnalyzeFile(const string& filename, bool showSwapped = false) {
    try {
        auto fileData = ReadFile(filename);
        
        if (fileData.size() < sizeof(XCOMPRESS_FILE_HEADER)) {
            cout << "File too small to contain valid header." << endl;
            return;
        }
        
        // Check file identifier
        XCOMPRESS_FILE_HEADER* header = reinterpret_cast<XCOMPRESS_FILE_HEADER*>(fileData.data());
        
        // Handle aliases
        DWORD correctIdentifier = header->Identifier;
        
        if (correctIdentifier == 0xEE12F50F) {
            correctIdentifier = XCOMPRESS_FILE_IDENTIFIER_LZXNATIVE;
        }
        else if (correctIdentifier == 0xED1250FF) {
            correctIdentifier = XCOMPRESS_FILE_IDENTIFIER_LZXTDECODE;
        }
        
        switch (correctIdentifier) {
            case XCOMPRESS_FILE_IDENTIFIER_LZXTDECODE:
                cout << "File uses LZXTDECODE compression format." << endl;
                if (!showSwapped) {
                    // For LZXTDECODE, we need to handle the structure properly
                    cout << "Note: LZXTDECODE format may require specific structure handling" << endl;
                }
                break;
            case XCOMPRESS_FILE_IDENTIFIER_LZXNATIVE:
                cout << "File uses LZXNATIVE compression format." << endl;
                if (fileData.size() >= sizeof(XCOMPRESS_FILE_HEADER_LZXNATIVE)) {
                    XCOMPRESS_FILE_HEADER_LZXNATIVE* lzxHeader = 
                        reinterpret_cast<XCOMPRESS_FILE_HEADER_LZXNATIVE*>(fileData.data());
                    
                    if (showSwapped) {
                        DisplayHeaderInfoSwapped(*lzxHeader);
                    } else {
                        DisplayHeaderInfo(*lzxHeader);
                    }
                }
                break;
            default:
                cout << "Unknown compression format (identifier: 0x" 
                     << hex << header->Identifier << dec << ")." << endl;
                return;
        }
        
        // Show file statistics
        cout << "\n=== File Statistics ===" << endl;
        cout << "Total File Size: " << fileData.size() << " bytes" << endl;
        cout << "File Name: " << filename << endl;
        
    } catch (const exception& e) {
        cerr << "Error analyzing file: " << e.what() << endl;
    }
}

// Function to display codec information
void DisplayCodecInfo() {
    cout << "=== Supported Compression Formats ===" << endl;
    cout << "XMEMCODEC_DEFAULT = 0" << endl;
    cout << "XMEMCODEC_LZX = 1" << endl;
    cout << "\n=== Compression Flags ===" << endl;
    cout << "XMEMCOMPRESS_STREAM = 0x00000001" << endl;
    cout << "\n=== File Identifiers ===" << endl;
    cout << "XCOMPRESS_FILE_IDENTIFIER_LZXTDECODE = 0x0FF512ED" << endl;
    cout << "XCOMPRESS_FILE_IDENTIFIER_LZXNATIVE = 0x0FF512EE" << endl;
    cout << "\n=== Version Information ===" << endl;
    cout << "LZXNATIVE version: " 
         << XCOMPRESS_LZXNATIVE_VERSION_MAJOR << "." 
         << XCOMPRESS_LZXNATIVE_VERSION_MINOR << endl;
}

int main(int argc, char* argv[]) {
    cout << "Xbox Compression File Analyzer" << endl;
    cout << "===============================" << endl;
    
    if (argc < 2) {
        cout << "\nUsage: " << argv[0] << " <compressed_file> [--swap]" << endl;
        cout << "Options:" << endl;
        cout << "  --swap   Display information with swapped (big-endian) byte order" << endl;
        cout << "  --info   Display codec information" << endl;
        return 1;
    }
    
    // Check for options
    bool showSwapped = false;
    
    if (string(argv[argc-1]) == "--swap") {
        showSwapped = true;
        argc--; // Remove the --swap argument from processing
    }
    
    if (string(argv[1]) == "--info") {
        DisplayCodecInfo();
        return 0;
    }
    
    // Analyze the specified file
    string filename = argv[1];
    cout << "Analyzing: " << filename << endl;
    
    AnalyzeFile(filename, showSwapped);
    
    return 0;
}