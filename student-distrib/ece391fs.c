#include "ece391fs.h"

ece391fs_bootblk_t* fs_bootblk = NULL;

/* int32_t ece391fs_init(uint32_t module_start, uint32_t module_end)
 * @input: module_start - start position of initramfs
 *         module_end - end position of initramfs
 * @output: variable fs_bootblk - set to module_start
 *          return value - SUCCESS / FAIL
 * @description: Verifies that the filesystem between module_start and module_end
 *     is a valid ECE391FS file system. If it is, put the start address to fs_bootblk
 *     global variable and return SUCCESS. If it isn't, return FAIL.
 */
int32_t ece391fs_init(uint32_t module_start, uint32_t module_end) {
    ece391fs_bootblk_t* fs_candidate = (ece391fs_bootblk_t*) module_start;
    // Check if the number of files is within MAX_FILE_COUNT
    if(fs_candidate->num_dir_entries > ECE391FS_MAX_FILE_COUNT) {
        return ECE391FS_CALL_FAIL;
    }
    // Check if the filesystem ends at correct address
    // The FS consists of 1 bootblock, *num_inodes* inodes, *num_data_blocks* data blocks.
    if((ece391fs_bootblk_t*) module_end
        != fs_candidate + (1 + fs_candidate->num_inodes + fs_candidate->num_data_blocks)) {
        return ECE391FS_CALL_FAIL;
    }
    // Register the filesystem globally
    fs_bootblk = fs_candidate;
    return ECE391FS_CALL_SUCCESS;
}

/* int32_t ece391fs_is_initialized()
 * @output: return value - SUCCESS / FAIL
 * @description: Return whether ece391fs is initialied.
 */
int32_t ece391fs_is_initialized() {
    return fs_bootblk ? ECE391FS_CALL_SUCCESS : ECE391FS_CALL_FAIL;
}

/* int32_t ece391fs_size(uint32_t inode_idx)
 * @input: inode_idx - index of inode whose size to be queried
 * @output: return value - file size in bytes of the inode
 *          FAIL - only if the inode doesn't exist
 * @description: query file size of given inode index.
 */
int32_t ece391fs_size(uint32_t inode_idx) {
    if(!fs_bootblk) return ECE391FS_CALL_FAIL;  // FS not initialized
    if(inode_idx >= fs_bootblk->num_inodes) return ECE391FS_CALL_FAIL;  // Index over inode count
    // Calculate pointer to the inode
    ece391fs_inode_t* inode = (ece391fs_inode_t*) fs_bootblk + (inode_idx + 1);
    return inode->size;
}

/* int32_t read_dentry_by_name(const char* fname, ece391fs_file_info_t* file_info)
 * @input: fname - filename
 *         file_info - file info struct to be written into
 * @output: file_info - filled with the information of queried file
 *          return value - PASS / FAIL
 * @description: query file info of given filename.
 *     "file info" is equivalent to "dentry" to make ece391 staff happy.
 */
int32_t read_dentry_by_name(const char* fname, ece391fs_file_info_t* file_info) {
    if(!fs_bootblk) return ECE391FS_CALL_FAIL;  // FS not initialized
    if(!file_info) return ECE391FS_CALL_FAIL;   // File info ptr invalid
    int i;
    for(i = 0; i < ECE391FS_MAX_FILE_COUNT; i++) {
        ece391fs_file_info_t* f = &(fs_bootblk->file[i]);
        int name_length = strlen(fname);
        if(name_length > ECE391FS_MAX_FILENAME_LEN) {
            name_length = ECE391FS_MAX_FILENAME_LEN;
        }
        if(0 == strncmp(fname, f->name, name_length)) {
            // This is the file we're looking for
            *file_info = *f;
            return ECE391FS_CALL_SUCCESS;
        }
    }
    // File not found
    return ECE391FS_CALL_FAIL;
}

/* int32_t read_dentry_by_index(uint32_t index, ece391fs_file_info_t* file_info)
 * @input: index - index of file inode to be queried.
 *         file_info - file info struct to be written into
 * @output: file_info - filled with the information of queried file
 *          return value - PASS / FAIL
 * @description: query file info of given index.
 *     "file info" is equivalent to "dentry" to make ece391 staff happy.
 */
int32_t read_dentry_by_index(uint32_t index, ece391fs_file_info_t* file_info) {
    if(!fs_bootblk) return ECE391FS_CALL_FAIL;  // FS not initialized
    if(index >= fs_bootblk->num_dir_entries) return ECE391FS_CALL_FAIL; // Index over inode count
    if(!file_info) return ECE391FS_CALL_FAIL;   // File info ptr invalid
    *file_info = fs_bootblk->file[index];
    return ECE391FS_CALL_SUCCESS;
}

/* int32_t read_data(uint32_t inode_idx, uint32_t offset, char* buf, uint32_t length)
 * @input: inode_idx - inode of file to be read from
 *         offset - starting byte from the beginning of file
 *         buf - where the data will be written into
 *         length - the number of bytes to be read
 * @output: buf - *length* bytes are written here
 *          return value - SUCCESS / FAIL
 * @description: read data from given inode.
 */
int32_t read_data(uint32_t inode_idx, uint32_t offset, char* buf, uint32_t length) {
    if(!fs_bootblk) return ECE391FS_CALL_FAIL;  // FS not initialized
    if(inode_idx >= fs_bootblk->num_inodes) return ECE391FS_CALL_FAIL;  // Index over inode count
    if(!buf) return ECE391FS_CALL_FAIL; // Buffer ptr invalid
    ece391fs_inode_t* inode = (ece391fs_inode_t*) fs_bootblk + (1 + inode_idx);
    if(offset >= inode->size) return ECE391FS_CALL_SUCCESS; // Offset over file length, nothing can be copied
    if(offset + length > inode->size) length = inode->size - offset;    // Length over end of file, reduce it

    // Calculate the block range where data will be read
    uint32_t first_data_block = offset / ECE391FS_BLOCK_SIZE;
    uint32_t last_data_block = (offset + length) / ECE391FS_BLOCK_SIZE;
    uint32_t bytes_done = 0;    // Counter of bytes copied, to determine the position of buf to write into
    uint32_t i;
    for(i = first_data_block; i <= last_data_block; i++) {
        // Find the actual data block
        uint32_t data_id = inode->data[i];
        ece391fs_data_block_t* data_ptr = (ece391fs_data_block_t*) fs_bootblk + (fs_bootblk->num_inodes + 1 + data_id);
        // Calculate the beginning and ending position of copying
        uint32_t block_byte_begin = 0;
        uint32_t block_byte_end = ECE391FS_BLOCK_SIZE;
        if(i * ECE391FS_BLOCK_SIZE < offset) {
            // This is the first block to be processed, offset should be applied.
            block_byte_begin = offset - i * ECE391FS_BLOCK_SIZE;
        }
        if((i + 1) * ECE391FS_BLOCK_SIZE > offset + length) {
            // This is the last block to be processed, length should be enforced.
            block_byte_end = offset + length - i * ECE391FS_BLOCK_SIZE;
        }
        memcpy((char*) buf + bytes_done, (char*) data_ptr + block_byte_begin, block_byte_end - block_byte_begin);
        bytes_done += block_byte_end - block_byte_begin;
    }
    return ECE391FS_CALL_SUCCESS;
}

void ece391fs_print_file_info(ece391fs_file_info_t* file_info) {
    int i;
    switch(file_info->type) {
        case ECE391FS_FILE_TYPE_RTC:
            printf("RTC"); break;
        case ECE391FS_FILE_TYPE_FOLDER:
            printf("Folder"); break;
        case ECE391FS_FILE_TYPE_FILE:
            printf("File"); break;
        default: break;
    }
    printf(" name: ");
    int name_length = strlen(file_info->name);
    if(name_length > ECE391FS_MAX_FILENAME_LEN) {
        name_length = ECE391FS_MAX_FILENAME_LEN;
    }
    for(i = 0; i < name_length; i++) {
        if(file_info->name[i]) {
            putc(file_info->name[i]);
        } else {
            break;
        }
    }
    printf("\n");
    if(file_info->type != ECE391FS_FILE_TYPE_FILE) return;
    printf("Inode #: %d \n", file_info->inode);
    ece391fs_inode_t* inode = (ece391fs_inode_t*) fs_bootblk + (file_info->inode + 1);
    printf("Size: %d \n", inode->size);
    printf("Blocks #: ");
    for(i = 0; i * ECE391FS_BLOCK_SIZE < inode->size; i++) {
        printf("%d ", inode->data[i]);
    }
    printf("\n");
}
