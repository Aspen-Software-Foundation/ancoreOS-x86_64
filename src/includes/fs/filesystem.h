#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>
#include <stddef.h>

/* ======================= Filesystem ======================= */
typedef struct filesystem {
    char label[144];               // Display name for user
    char id_str[16];               // e.g., "FAT32", "exFAT", "NTFS", "ISO9660", etc
    char part_guid[16];            // GUID for this partition
    char type_guid[16];            // GUID for this filesystem
    uint64_t start_lba;            // first sector of this FS/partition
    uint64_t length;               // in sectors
    void *ctx;                     // pointer to FAT/GPT/etc context. must be CASTed
    uint32_t fs_flags;             // BOOT, RECOVERY, EFI, INTERNAL_STORAGE, etc
} filesystem_t;

/* ==================== Filesystem List ==================== */
typedef struct filesystem_list {
    size_t fs_count;               // number of filesystems in the list
    filesystem_t *fs_entries;      // pointer to array of filesystems
} filesystem_list_t;


#endif // FILESYSTEM_H
