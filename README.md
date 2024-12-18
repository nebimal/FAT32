# FAT32 File System Shell Utility  

This project implements a custom shell utility to interact with a FAT32 file system image. The application operates entirely in user space, providing robust functionality for managing files and directories without modifying the original disk image.  

---

## Features  

- **File Management:**  
  Open, close, save, retrieve, store, delete, and restore files in a FAT32 image.  

- **Directory Operations:**  
  Navigate directories using relative or absolute paths and list directory contents.  

- **File System Information:**  
  Display key metadata, such as bytes per sector, sectors per cluster, and file system structure.  

- **Data Reading:**  
  Read file content in multiple formats (hexadecimal, ASCII, or decimal).  

- **Error Handling:**  
  Graceful handling of invalid commands and scenarios like accessing a closed file system.  

---

## Commands Overview  

- **File System Management:**  
  - `open <filename>`: Open a FAT32 image.  
  - `close`: Close the current FAT32 image.  
  - `info`: Display FAT32 metadata.  

- **File Operations:**  
  - `get <filename> [new filename]`: Retrieve a file from the FAT32 image.  
  - `put <filename> [new filename]`: Add a file to the FAT32 image.  
  - `del <filename>`: Delete a file from the image.  
  - `undel <filename>`: Restore a deleted file.  
  - `stat <filename>`: Display attributes and cluster details.  

- **Navigation:**  
  - `cd <directory>`: Change the working directory.  
  - `ls`: List directory contents.  

- **Data Reading:**  
  - `read <filename> <position> <number of bytes> [option]`: Read file content. Options include:  
    - `-ascii`: Print as ASCII characters.  
    - `-dec`: Print as decimal values.  

- **Exit Commands:**  
  - `quit` or `exit`: Close the program.  

---

## Technical Details  

- **Programming Language:** C  
- **Program Name:** `mfs.c`  
- **Key Restrictions:**  
  - No `system()`, `fork()`, or `exec` system calls.  
  - Disk image mounting is not allowed.  
  - Operates entirely in memory.  
- **Case Insensitivity:** All commands are case insensitive.  

---

## Included Files  

- **`fat32.img`**: Sample FAT32 file system image for testing.  
- **`fatspec.pdf`**: FAT32 file system specification reference.  
- **`mfs.c`**: Source code for the shell utility.  

---

## How to Run  

1. Compile the program:  
   ```bash
     make
2. Run the program:
   ```bash
    ./mfs

# Acknowledgments
This project was completed as part of a file systems course assignment. I would like to thank my instructor for providing the opportunity to deepen my understanding of FAT32, file allocation tables, and endianness. The detailed specifications and constraints of the project enhanced my problem-solving skills and my ability to write robust C code.

Feel free to explore the code, and contributions or feedback are always welcome!
