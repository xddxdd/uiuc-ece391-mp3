#include "ece391fs.h"

ece391fs_bootblk_t* fs_bootblk = NULL;

int32_t ece391fs_init(uint32_t module_start, uint32_t module_end) {
    fs_bootblk = (ece391fs_bootblk_t*) module_start;
    return 0;
}

int32_t ece391fs_size(uint32_t inode_idx) {
    if(inode_idx >= fs_bootblk->num_inodes) return -1;
    ece391fs_inode_t* inode = (ece391fs_inode_t*) fs_bootblk + (inode_idx + 1);
    return inode->size;
}

int32_t read_dentry_by_name(const char* fname, ece391fs_file_info_t* file_info) {
    if(!fs_bootblk) return -1;
    if(!file_info) return -1;
    int i;
    for(i = 0; i < ECE391FS_MAX_FILE_COUNT; i++) {
        ece391fs_file_info_t* f = &(fs_bootblk->file[i]);
        int name_length = strlen(fname);
        if(0 == strncmp(fname, f->name, name_length)) {
            *file_info = *f;
            return 0;
        }
    }
    return -1;
}

int32_t read_dentry_by_index(uint32_t index, ece391fs_file_info_t* file_info) {
    if(!fs_bootblk) return -1;
    if(index >= fs_bootblk->num_dir_entries) return -1;
    if(!file_info) return -1;
    *file_info = fs_bootblk->file[index];
    return 0;
}

int32_t read_data(uint32_t inode_idx, uint32_t offset, char* buf, uint32_t length) {
    if(!fs_bootblk) return -1;
    if(inode_idx >= fs_bootblk->num_inodes) return -1;
    if(!buf) return -1;
    ece391fs_inode_t* inode = (ece391fs_inode_t*) fs_bootblk + (1 + inode_idx);
    if(offset >= inode->size) return 0;
    if(offset + length > inode->size) {
        length = inode->size - offset;
    }

    uint32_t first_data_block = offset / ECE391FS_BLOCK_SIZE;
    uint32_t last_data_block = (offset + length) / ECE391FS_BLOCK_SIZE;
    uint32_t bytes_done = 0;
    uint32_t i;
    for(i = first_data_block; i <= last_data_block; i++) {
        uint32_t data_id = inode->data[i];
        ece391fs_data_block_t* data_ptr = (ece391fs_data_block_t*) fs_bootblk + (fs_bootblk->num_inodes + 1 + data_id);
        uint32_t block_byte_begin = i * ECE391FS_BLOCK_SIZE;
        uint32_t block_byte_end = block_byte_begin + ECE391FS_BLOCK_SIZE;
        if(block_byte_begin < offset) {
            block_byte_begin = offset;
        }
        if(block_byte_end > offset + length) {
            block_byte_end = offset + length;
        }
        block_byte_begin -= i * ECE391FS_BLOCK_SIZE;
        block_byte_end -= i * ECE391FS_BLOCK_SIZE;
        memcpy(buf + bytes_done, data_ptr + block_byte_begin, block_byte_end - block_byte_begin);
    }
    return 0;
}

void print_file_info(ece391fs_file_info_t* file_info) {
    switch(file_info->type) {
        case ECE391FS_FILE_TYPE_RTC:
            printf("RTC"); break;
        case ECE391FS_FILE_TYPE_FOLDER:
            printf("Folder"); break;
        case ECE391FS_FILE_TYPE_FILE:
            printf("File"); break;
        default: break;
    }
    printf(" name: %s \n", file_info->name);
    if(file_info->type != ECE391FS_FILE_TYPE_FILE) return;
    printf("Inode #: %d \n", file_info->inode);
    ece391fs_inode_t* inode = (ece391fs_inode_t*) fs_bootblk + (file_info->inode + 1);
    printf("Size: %d \n", inode->size);
    printf("Blocks #: ");
    int i;
    for(i = 0; i * ECE391FS_BLOCK_SIZE < inode->size; i++) {
        printf("%d ", inode->data[i]);
    }
    printf("\n");
}
