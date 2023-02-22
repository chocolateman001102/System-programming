/**
 * finding_filesystems
 * CS 341 - Fall 2022
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Virtual paths:
 *  Add your new virtual endpoint to minixfs_virtual_path_names
 */
char *minixfs_virtual_path_names[] = {"info", /* add your paths here*/};

/**
 * Forward declaring block_info_string so that we can attach unused on it
 * This prevents a compiler warning if you haven't used it yet.
 *
 * This function generates the info string that the virtual endpoint info should
 * emit when read
 */
static char *block_info_string(ssize_t num_used_blocks) __attribute__((unused));
static char *block_info_string(ssize_t num_used_blocks) {
    char *block_string = NULL;
    ssize_t curr_free_blocks = DATA_NUMBER - num_used_blocks;
    asprintf(&block_string,
             "Free blocks: %zd\n"
             "Used blocks: %zd\n",
             curr_free_blocks, num_used_blocks);
    return block_string;
}

// Don't modify this line unless you know what you're doing
int minixfs_virtual_path_count =
    sizeof(minixfs_virtual_path_names) / sizeof(minixfs_virtual_path_names[0]);

int minixfs_chmod(file_system *fs, char *path, int new_permissions) {

    inode *idx_node = get_inode(fs, path);
    if (!idx_node) {
      errno = ENOENT;
      return -1;
    }
    if (idx_node != NULL) {
        idx_node->mode = new_permissions | ((idx_node->mode >> RWX_BITS_NUMBER) << RWX_BITS_NUMBER);
        clock_gettime(CLOCK_REALTIME, &idx_node->ctim);
        return 0;
    }
    return -1;
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
    inode *idx_node = get_inode(fs, path);
    if (!idx_node) {
      errno = ENOENT;
      return -1;
    }

    if (idx_node != NULL) {

        if(owner != (uid_t)-1) {
            idx_node->uid =owner;
        }
        if(group != (uid_t)-1) {
            idx_node->gid = group;
        }
        clock_gettime(CLOCK_REALTIME, &idx_node->ctim);
        return 0;
    }

    return -1;
}




inode *minixfs_create_inode_for_path(file_system *fs, const char *path) {

    if (get_inode(fs, path))  {
        return NULL;
    }

    if(!valid_filename(path)) {
        return NULL;
    }
    inode_number i_number = first_unused_inode(fs);
    if(i_number == -1) {
        return NULL;
    }

    const char *filename;
    inode *parent = parent_directory(fs, path, &filename);

    init_inode(parent, & (fs -> inode_root[i_number]));

    minixfs_dirent dire_file;
    dire_file.name = strdup(filename);
    dire_file.inode_num = i_number;


    int block_idx = (parent -> size) / sizeof(data_block);
    size_t block_offset = (parent->size) % sizeof(data_block);
    data_block_number db_number;

    if (block_idx < NUM_DIRECT_BLOCKS) {

      db_number = parent -> direct[block_idx];
      make_string_from_dirent(fs -> data_root[db_number].data + block_offset, dire_file);

    } else if(block_idx >= NUM_DIRECT_BLOCKS) {

      db_number = parent->indirect;
      data_block_number ind_idx = *( (data_block_number *) fs -> data_root[db_number].data + (block_idx - NUM_DIRECT_BLOCKS) * sizeof(data_block_number) );
      make_string_from_dirent(fs->data_root[ind_idx].data + block_offset, dire_file);

    }

    free(dire_file.name);
    return fs->inode_root + i_number;
}

ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {
    if (!strcmp(path, "info")) {
        char* data_map = GET_DATA_MAP(fs->meta);
        ssize_t num = 0;
        for (uint64_t i = 0; i < fs->meta->dblock_count; i++) {
            if ( data_map[i] == 1) {
                ++num;
            }
        }
        char* b_info = block_info_string(num);
        size_t length = strlen(b_info);
        if (((int)length) < *off) {
            return 0;
        }

        if (length - *off < count) {
            count = strlen(b_info - *off);
        }

        memmove(buf, b_info + *off, count);
        *off = *off + count;
        return count;
    }
    errno = ENOENT;
    return -1;
}




ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
     
    inode *idx_node = get_inode(fs, path);
    size_t block_num_test = (*off + count) / sizeof(data_block);
    if (idx_node == NULL) {
        idx_node = minixfs_create_inode_for_path(fs, path);
        if (idx_node == NULL) {
            errno = ENOSPC;
            return -1;
        }
    }

    if (count + *off > (NUM_DIRECT_BLOCKS + NUM_INDIRECT_BLOCKS) * sizeof(data_block)) {
      errno = ENOSPC;
      return -1;
    }

    if (minixfs_min_blockcount(fs, path, (int)block_num_test) == -1) {
        errno = ENOSPC;
        return -1;
    }


    size_t block_idx = *off / sizeof(data_block);
    size_t block_offset = *off % sizeof(data_block);

    data_block_number data_block_num;


    size_t written_bytes = 0;
    while (written_bytes < count) {
       // printf("%zu block_idx \n",block_idx);
      if (block_idx < NUM_DIRECT_BLOCKS) {
       // printf("%d direct data_block_num \n",data_block_num);
        data_block_num = idx_node -> direct[block_idx];
      } else if(block_idx >= NUM_DIRECT_BLOCKS){
        //printf("I am doing indiect writing !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
        //printf("%d idx_node->indirect \n",idx_node->indirect);
        data_block_num = *((data_block_number *)(fs->data_root + idx_node->indirect)) + (block_idx - NUM_DIRECT_BLOCKS);
        //printf("%d indirect data_block_num \n",data_block_num);

      }

      size_t each_write = 0;
      if(count - written_bytes > sizeof(data_block) - block_offset) {
        each_write = sizeof(data_block) - block_offset;
      } else {
        each_write = count - written_bytes;
      }

      memcpy(fs -> data_root[data_block_num].data + block_offset, buf + written_bytes, each_write);

      written_bytes += each_write;
      //printf("%zu written_bytes \n",written_bytes);
      block_offset = 0;
      block_idx++;
    }
    *off += count;
    idx_node->size = *off;
    clock_gettime(CLOCK_REALTIME, &idx_node->mtim);
    clock_gettime(CLOCK_REALTIME, &idx_node->atim);
    return count;
}





ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path)
        return minixfs_virtual_read(fs, virtual_path, buf, count, off);
    // 'ere be treasure!
    
    inode *idx_node = get_inode(fs, path);
    if (idx_node == NULL) {
        idx_node = minixfs_create_inode_for_path(fs, path);
        if (idx_node == NULL) {
            errno = ENOSPC;
            return -1;
        }
    }
    size_t block_num_test = (*off + count) / sizeof(data_block);
    if ((*off + count) % sizeof(data_block) != 0) {
        ++block_num_test;
    }

    int block_idx = *off / sizeof(data_block);
    int block_offset = *off % sizeof(data_block);
    data_block_number data_block_num;
    size_t read_bytes = 0;

    if(count > idx_node->size - *off ) {
        count = idx_node->size - *off ;
    }

    while (read_bytes < count) {
        if (block_idx < NUM_DIRECT_BLOCKS) {
        data_block_num = idx_node->direct[block_idx];
        } else if(block_idx >= NUM_DIRECT_BLOCKS) {
            //printf("%d data_block_num \n",data_block_num);
        data_block_num = *((data_block_number *)(fs->data_root + idx_node->indirect)) + (block_idx - NUM_DIRECT_BLOCKS);
        }
        
        size_t bytes_each_read = 0;
        if(count - read_bytes > sizeof(data_block)) {
            bytes_each_read = sizeof(data_block) - block_offset;
        } else {
            bytes_each_read = count - read_bytes;
        }
        memcpy(buf + read_bytes, fs -> data_root[data_block_num].data + block_offset, bytes_each_read);
        read_bytes += bytes_each_read;
        block_offset = 0;
        block_idx++;
    }
    *off += read_bytes;
    clock_gettime(CLOCK_REALTIME, &(idx_node->atim));
    return read_bytes;
}
