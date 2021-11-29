// fs.cpp: File System

#include "sfs/fs.h"

// #include <algorithm>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define min(X,Y) (((X) < (Y)) ? (X) : (Y))
#define max(X,Y) (((X) > (Y)) ? (X) : (Y))

//Global Variables
uint32_t numBlocks;
uint32_t numInodeBlocks;
uint32_t numInodes;
Disk* diskS;
uint32_t *bitmap;

//Auxiliary Functions
bool saveInode(uint32_t inodeNum, Inode *inode);
bool loadInode(uint32_t inodeNum, Inode *inode);
ssize_t allocateFreeBlock();
float ceiling(float x);

// Debug file system -----------------------------------------------------------

void debug(Disk *disk) {
    Block block;

    // Read Superblock

    disk->readDisk(disk, 0, block.Data);

    printf("SuperBlock:\n");
    printf("    magic number is %s\n", (block.Super.MagicNumber == MAGIC_NUMBER ? "valid":"invalid"));
    printf("    %u blocks\n"         , block.Super.Blocks);
    printf("    %u inode blocks\n"   , block.Super.InodeBlocks);
    printf("    %u inodes\n"         , block.Super.Inodes);


    // Read Inode blocks
    int inodeBlocks = block.Super.InodeBlocks;
    for(int inodeBlock = 1; inodeBlock <= inodeBlocks; inodeBlock++){
        disk->readDisk(disk,inodeBlock,block.Data);
        for(unsigned int curInode = 0; curInode < INODES_PER_BLOCK; curInode++){
            Inode inode = block.Inodes[curInode];
            if(!inode.Valid)
                continue;
            printf("Inode %d:\n", curInode);
            printf("    size: %u bytes\n",inode.Size);
            printf("    direct blocks:");
            for(unsigned int directBlock = 0; directBlock < POINTERS_PER_INODE; directBlock++){
                unsigned int directBlockNumber = inode.Direct[directBlock];
                if(directBlockNumber == 0) continue;
                printf(" %u", directBlockNumber);
            }
            printf("\n");
            unsigned int indirectPointer = inode.Indirect;
            // we begin with first indirect
            if(indirectPointer) {
                Block indirectBlock;
                printf("    indirect block: %u\n", indirectPointer);
                printf("    indirect data blocks:");
                disk->readDisk(disk, indirectPointer, indirectBlock.Data);
                for(unsigned int indBlockNum = 0; indBlockNum < POINTERS_PER_BLOCK; indBlockNum++){
                    indirectPointer = indirectBlock.Pointers[indBlockNum];
                    if(indirectPointer == 0 || indirectPointer > POINTERS_PER_BLOCK) continue;
                    printf(" %u", indirectPointer);
                }
                printf("\n");
            }
            unsigned int doubleIndirectPointer = inode.DoubleIndirect;
            if(doubleIndirectPointer) {
                Block doubleIndirectBlock;
                printf("    double indirect block: %u\n", doubleIndirectPointer);
                printf("    double indirect data blocks:");
                disk->readDisk(disk, doubleIndirectPointer, doubleIndirectBlock.Data);
                for(unsigned int doubleIndBlockNum = 0; doubleIndBlockNum < POINTERS_PER_BLOCK; doubleIndBlockNum++){

                    doubleIndirectPointer = doubleIndirectBlock.Pointers[doubleIndBlockNum];
                    if(doubleIndirectPointer == 0 || doubleIndirectPointer > POINTERS_PER_BLOCK) continue;

                    if(doubleIndirectPointer) {
                        Block singleIndirectBlock;
                        unsigned int indPtr;
                        disk->readDisk(disk, doubleIndirectPointer, singleIndirectBlock.Data);
                        for (unsigned int indBlockNum = 0; indBlockNum < POINTERS_PER_BLOCK; indBlockNum++) {
                            indPtr = singleIndirectBlock.Pointers[indBlockNum];
                            if (indPtr == 0 || indPtr > POINTERS_PER_BLOCK) continue;
                            printf(" %u", indPtr);
                        }
                    }
                }
                printf("\n");
            }
        }
    }
}

// Format file system ----------------------------------------------------------

bool format(Disk *disk) {
    // Check if mounted
    if (disk->mounted(disk)) return false;

    // Write content to superblock
    Block block;
    memset(block.Data, 0, BLOCK_SIZE);
    block.Super.MagicNumber = MAGIC_NUMBER;
    block.Super.Blocks = disk->Blocks;
    block.Super.InodeBlocks = (uint32_t)(((float) disk->Blocks * 0.1) + 0.5);
    block.Super.Inodes = INODES_PER_BLOCK * block.Super.InodeBlocks;
    disk->writeDisk(disk, 0, block.Data);

    // Clear all other blocks
    char clearBlock[BLOCK_SIZE] = {0};
    for (int i = 1; i < block.Super.Blocks; i++) disk->writeDisk(disk, i, clearBlock);

    return true;
}

// Mount file system -----------------------------------------------------------

bool mount(Disk *disk) {
    if(disk->mounted(disk)) return false;

    // Read superblock
    Block block;
    disk->readDisk(disk, 0, block.Data);

    // Set device and mount
    if(block.Super.Inodes != block.Super.InodeBlocks * INODES_PER_BLOCK ||
       block.Super.MagicNumber != MAGIC_NUMBER || block.Super.Blocks < 0 ||
       block.Super.InodeBlocks != ceiling(0.1 * block.Super.Blocks)){
        return false;
    }

    disk->mount(disk);

    // Copy metadata
    numBlocks = block.Super.Blocks;
    numInodeBlocks = block.Super.InodeBlocks;
    numInodes = block.Super.Inodes;
    diskS = disk;

    // Allocate free block bitmap
    bitmap = (uint32_t *) calloc(numBlocks, sizeof(uint32_t));

    //Init all blocks to free initally
    for (uint32_t i = 0; i < numBlocks; i++) bitmap[i] = 1;

    bitmap[0] = 0; //Superblock is not free

    // inode blocks are not free
    for (uint32_t i = 0; i < numInodeBlocks; i++) bitmap[1 + i] = 0;

    //read inodes to determine which blocks are free
    for (uint32_t inode_block = 0; inode_block < numInodeBlocks; inode_block++) {
        Block b;
        disk->readDisk(disk, 1 + inode_block, b.Data);
        // reads each inode
        for (uint32_t inode = 0; inode < INODES_PER_BLOCK; inode++) {
            // if it's not valid, it has no blocks
            if (!b.Inodes[inode].Valid) continue;

            uint32_t n_blocks = (uint32_t) ceiling(b.Inodes[inode].Size / (float) BLOCK_SIZE);
            // read all direct blocks
            for (uint32_t pointer = 0; pointer < POINTERS_PER_INODE && pointer < n_blocks; pointer++) {
                bitmap[b.Inodes[inode].Direct[pointer]] = 0;
            }

            if (n_blocks > POINTERS_PER_INODE) {
                //read indirect block if necessary
                Block indirect;
                disk->readDisk(disk, b.Inodes[inode].Indirect, indirect.Data);
                bitmap[b.Inodes[inode].Indirect] = 0;
                for (uint32_t pointer = 0; pointer < n_blocks - POINTERS_PER_INODE; pointer++) {
                    bitmap[indirect.Pointers[pointer]] = 0;
                }

                //read double indirect block if necessary
                Block doubleIndirect;
                disk->readDisk(disk, b.Inodes[inode].DoubleIndirect, doubleIndirect.Data);
                bitmap[b.Inodes[inode].DoubleIndirect] = 0;
                for (uint32_t pointer = 0; pointer < n_blocks - POINTERS_PER_INODE; pointer++) {
                    Block indirectBlock;
                    disk->readDisk(disk, doubleIndirect.Inodes[pointer].Indirect, indirectBlock.Data);
                    for (uint32_t ptr = 0; ptr < n_blocks - POINTERS_PER_INODE; ptr++) {
                        bitmap[indirectBlock.Pointers[ptr]] = 0;
                    }
                }
            }
        }
    }
    return true;
}

// Create inode ----------------------------------------------------------------

size_t create() {
    // Locate free inode in inode table
    int inodeNum = -1;
    if(!diskS->mounted(diskS)) return -1; //If disk is not mounted, we cannot store new Inodes

    for (uint32_t inodeBlock = 0; inodeBlock < numInodeBlocks; inodeBlock++) {
        Block b;
        diskS->readDisk(diskS, inodeBlock + 1, b.Data); //Read each Inode block on disk

        //Read each inode on inode block
        for (unsigned int inode = 0; inode < INODES_PER_BLOCK; inode++) {
            //If we find an invalid inode in a certain block, it means we found a free spot to create our inode
            if(!b.Inodes[inode].Valid){
                //We change our result to be the location of the free Inode in the block and we finish
                inodeNum = inode + INODES_PER_BLOCK * inodeBlock;
                break;
            }
        }

        //If we exit the loop and our result is still -1, we did not find a free spot for our new Inode
        if(inodeNum != -1) break;
    }

    //If no inode is invalid, meaning it's not free to use, then return an error
    if(inodeNum == -1) return -1;
    else {
        // If there is a free spot, record the inode

        //Create empty inode to store and use later on
        Inode inode;
        inode.Valid = 1;
        inode.Size = 0;

        //Set all direct pointers to 0 to use later
        for (uint32_t i = 0; i < POINTERS_PER_INODE; i++) inode.Direct[i] = 0;

        //Set indirect pointer to 0 to use later on
        inode.Indirect = 0;
        inode.DoubleIndirect = 0;

        //Save Inode on disk
        saveInode(inodeNum, &inode);
    }


    return inodeNum;
}

/*****************AUXILIARY FUNCTIONS*************************/

bool saveInode(uint32_t inodeNum, Inode *inode) {

    uint32_t blockNumber = 1 + inodeNum / INODES_PER_BLOCK;
    uint32_t inodeOffset = inodeNum % INODES_PER_BLOCK;

    if (inodeNum >= numInodes) return false;

    Block block;
    diskS->readDisk(diskS, blockNumber, block.Data);
    block.Inodes[inodeOffset] = *inode;
    diskS->writeDisk(diskS, blockNumber, block.Data);

    return true;
}

bool loadInode(uint32_t inodeNum, Inode *inode) {
    uint32_t blockNumber = 1 + (inodeNum / INODES_PER_BLOCK);
    uint32_t inodeOffset = inodeNum % INODES_PER_BLOCK;

    if (inodeNum >= numInodes) {
        return false;
    }

    Block block;
    diskS->readDisk(diskS, blockNumber, block.Data);

    *inode = block.Inodes[inodeOffset];

    return true;
}

ssize_t allocateFreeBlock() {
    int block = -1;
    for (unsigned int i = 0; i < numBlocks; i++) {
        if (bitmap[i]) {
            bitmap[i] = 0;
            block = i;
            break;
        }
    }

    // need to zero data block if we're allocating one
    if (block != -1) {
        char data[BLOCK_SIZE];
        memset(data, 0, BLOCK_SIZE);
        diskS->writeDisk(diskS, block,(char*) data);
    }

    return block;
}

union float_int {
    float f;
    int i;
};

float ceiling(float x) {
    union float_int val;
    val.f = x;
    int sign = val.i >> 31;
    int exponent = ((val.i & 0x7fffffff) >> 23) - 127;
    int mantissa = val.i & 0x7fffff;

    if(exponent < 0){
        if(x <= 0.0f) return 0.0f;
        else return 1.0f;
    }
    else{
        int mask=0x7fffff >> exponent;

        if((mantissa & mask) == 0) return x;
        else{
            if(!sign){
                mantissa += 1 << (23 - exponent);

                if(mantissa & 0x800000){
                    mantissa = 0;
                    exponent++;
                }
            }
            mantissa &= ~mask;
        }
    }

    val.i = (sign << 31) | ((exponent + 127) << 23) | mantissa;

    return val.f;
}

/*****************AUXILIARY FUNCTIONS*************************/

// Remove inode ----------------------------------------------------------------

bool removeInode(size_t inumber) {
    Inode inode;

    // Load inode information
    if(!loadInode(inumber, &inode)) return false;
    if(!inode.Valid) return false;

    // Free direct blocks
    for(unsigned int i = 0; i < POINTERS_PER_INODE; i++){
        if(inode.Direct[i] != 0){
            bitmap[inode.Direct[i]] = 1; //Un-occupy place in bit map
            inode.Direct[i] = 0;
        }
    }

    // Free indirect blocks
    if(inode.Indirect != 0){
        bitmap[inode.Indirect] = 1; //Un-Occupy place in bit map
        Block block;
        diskS->readDisk(diskS, inode.Indirect, block.Data);
        //Free blocks pointed to indirectly
        for(unsigned int i = 0; i < POINTERS_PER_BLOCK; i++){
            if(block.Pointers[i] != 0){
                bitmap[block.Pointers[i]] = 1; //Un-occupy place in bit map
            }
        }
    }

    //Free Double Indirect Blocks
    if(inode.DoubleIndirect != 0){
        bitmap[inode.DoubleIndirect] = 1; //Un-Occupy place in bit map
        Block block;
        Block indirect;
        diskS->readDisk(diskS, inode.DoubleIndirect, block.Data);
        //Free blocks pointed to indirectly
        for(unsigned int i = 0; i < POINTERS_PER_BLOCK; i++){
            if(block.Inodes[i].Indirect != 0){
                diskS->readDisk(diskS, block.Inodes[i].Indirect, indirect.Data);
                for(unsigned int j = 0; j < POINTERS_PER_BLOCK; j++) {
                    bitmap[indirect.Pointers[j]] = 1; //Un-occupy place in bit map
                }
            }
        }
    }

    // Clear inode in inode table
    inode.Valid = 0;
    inode.Indirect = 0;
    inode.DoubleIndirect = 0;
    inode.Size = 0;
    if(!saveInode(inumber, &inode)) return false;

    return true;
}

// Inode stat ------------------------------------------------------------------

size_t stat(size_t inumber) {
    // Load inode information
    Inode inode;
    if(!loadInode(inumber, &inode)) return -1;
    if(!inode.Valid) return -1;

    return inode.Size;
}

// Read from inode -------------------------------------------------------------

size_t readInode(size_t inumber, char *data, size_t length, size_t offset) {
    // Load inode information
    Inode inode;
    if(!loadInode(inumber, &inode) || offset > inode.Size) return -1;

    // Adjust length
    length = min(length, inode.Size - offset);

    uint32_t startBlock = offset / BLOCK_SIZE;

    // Read block and copy to data
    Block indirect;
    if((offset + length) / BLOCK_SIZE > POINTERS_PER_INODE){
        if(inode.Indirect == 0) return -1; //Make sure direct block is allocated
        diskS->readDisk(diskS, inode.Indirect, indirect.Data);
    }

    size_t read = 0;
    for (uint32_t block = startBlock; read < length; block++) {

        //Find which block we're reading from
        size_t blockToRead = (block < POINTERS_PER_INODE) ? inode.Direct[block] : indirect.Pointers[block - POINTERS_PER_INODE];

        if(blockToRead == 0) return -1; //Make sure block is allocated

        //Get the block (from either direct or indirect)
        Block b;
        diskS->readDisk(diskS, blockToRead, b.Data);
        size_t readOffset;
        size_t readLength;

        // if it's the first block read, have to start from an offset
        // and read either until the end of the block, or the whole request
        if(read == 0){
            readOffset = offset % BLOCK_SIZE;
            readLength = min(BLOCK_SIZE - readOffset, length);
        } else {
            // otherwise, start from the beginning, and read
            // either the whole block or the rest of the request
            readOffset = 0;
            readLength = min(BLOCK_SIZE - 0, length - read);
        }

        memcpy(data + read, b.Data + readOffset, readLength);
        read += readLength;

    }
    return read;
}

// Write to inode --------------------------------------------------------------

size_t writeInode(size_t inumber, char *data, size_t length, size_t offset) {
    // Load inode
    Inode inode;
    if(!loadInode(inumber, &inode) || offset > inode.Size) return -1;

    size_t MAX_FILE_SIZE = BLOCK_SIZE * (POINTERS_PER_INODE * POINTERS_PER_BLOCK);

    //Adjust Length
    length = min(length, MAX_FILE_SIZE - offset);
    uint32_t startBlock = offset / BLOCK_SIZE;
    Block indirect;
    Block doubleIndirect;

    bool readIndirect = false;
    bool readDoubleIndirect = false;
    bool modifiedInode = false;
    bool modifiedIndirect = false;
    bool modifiedDoubleIndirect = false;

    // Write block and copy to data
    size_t written = 0;
    for (uint32_t block = startBlock; written < length && block < POINTERS_PER_INODE + POINTERS_PER_BLOCK; block++) {
        size_t blockToWrite;
        if(block < POINTERS_PER_INODE){
            //Direct Block
            //Allocate block if necessary
            if(inode.Direct[block] == 0){
                ssize_t allocated_block = allocateFreeBlock();
                if(allocated_block == -1) break;
                inode.Direct[block] = allocated_block;
                modifiedInode = true;
            }
            blockToWrite = inode.Direct[block];
        } else {
            //Indirect/Double Indirect Block

            //Allocate Indirect block if necessary
            if(inode.Indirect == 0){
                ssize_t allocatedBlock = allocateFreeBlock();
                if(allocatedBlock == -1) return written;
                inode.Indirect = allocatedBlock;
                modifiedIndirect = true;
            } else if(inode.DoubleIndirect == 0){
                ssize_t allocatedBlock = allocateFreeBlock();
                if(allocatedBlock == -1) return written;
                inode.DoubleIndirect = allocatedBlock;
                modifiedDoubleIndirect = true;
            }

            //Read indirect block if it hasn't been read yet
            if(!readIndirect){
                diskS->readDisk(diskS, inode.Indirect, indirect.Data);
                readIndirect = true;
            } else if(!readDoubleIndirect){
                diskS->readDisk(diskS, inode.DoubleIndirect, doubleIndirect.Data);
                readDoubleIndirect = true;
            }

            //Allocate block if necessary
            if(indirect.Pointers[block - POINTERS_PER_INODE] == 0){
                ssize_t allocatedBlock = allocateFreeBlock();
                if(allocatedBlock == -1) break;
                indirect.Pointers[block - POINTERS_PER_INODE] = allocatedBlock;
                modifiedIndirect = true;
            } else if(doubleIndirect.Inodes[block - POINTERS_PER_INODE].Indirect == 0){
                Block indirectBlock;
                ssize_t allocatedBlock = allocateFreeBlock();
                if(allocatedBlock == -1) break;
                uint32_t dIndPtr = doubleIndirect.Pointers[block - POINTERS_PER_INODE];
                diskS->readDisk(diskS, dIndPtr, indirectBlock.Data);
                indirect.Pointers[block - POINTERS_PER_INODE] = allocatedBlock;
                modifiedDoubleIndirect = true;
            }

            if(modifiedDoubleIndirect){
                Block indirectBlock;
                uint32_t dIndPtr = doubleIndirect.Pointers[block - POINTERS_PER_INODE];
                diskS->readDisk(diskS, dIndPtr, indirectBlock.Data);
                blockToWrite = indirectBlock.Pointers[block - POINTERS_PER_INODE];
            } else {                 
                blockToWrite = indirect.Pointers[block - POINTERS_PER_INODE];
            }

        }

        //Get the block (either direct or indirect)
        size_t writeOffset;
        size_t writeLength;

        // if it's the first block written, have to start from an offset
        // and write either until the end of the block, or the whole request
        if(written == 0){
            writeOffset = offset % BLOCK_SIZE;
            writeLength = min(BLOCK_SIZE - writeOffset, length);
        } else {
            // otherwise, start from the beginning, and write
            // either the whole block or the rest of the request
            writeOffset = 0;
            writeLength = min(BLOCK_SIZE - 0, length - written);
        }

        char writeBuffer[BLOCK_SIZE];

        // if we're not writing the whole block, need to copy what's there
        if(writeLength < BLOCK_SIZE)
            diskS->readDisk(diskS, blockToWrite, writeBuffer);

        //Copy into buffer
        memcpy(writeBuffer + writeOffset, data + written, writeLength);
        diskS->writeDisk(diskS, blockToWrite, (char *) writeBuffer);
        written += writeLength;
    }

    //Update Inode size if Inode was modified
    uint32_t newSize = max((size_t) inode.Size, written + offset);
    if(newSize != inode.Size){
        inode.Size = newSize;
        modifiedInode = true;
    }

    //Save modifications on indirect and inode, if any
    if(modifiedInode) saveInode(inumber, &inode);
    if(modifiedIndirect) diskS->writeDisk(diskS, inode.Indirect, indirect.Data);
    if(modifiedIndirect) diskS->writeDisk(diskS, inode.DoubleIndirect, doubleIndirect.Data);


    return written;
}
