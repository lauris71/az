#define __AZ_HASH_MAP_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdlib.h>
#include <string.h>

#include <az/value.h>

#include "hash-map.h"

struct _AZHashMapEntry {
    uint32_t next;
};

#define EMPTY 0
#define END 1

/* Internal helpers */
static AZHashMapEntry *
entry_ptr(AZHashMapImplementation *impl, AZHashMapEntry *entries, unsigned int pos)
{
	return (AZHashMapEntry *) ((char *) entries + pos * impl->entry_size);
}

static AZValue *
key_ptr(AZHashMapImplementation *impl, AZHashMapEntry *entry)
{
	return (AZValue *) ((char *) entry + impl->key_offset);
}

static AZValue *
val_ptr(AZHashMapImplementation *impl, AZHashMapEntry *entry)
{
	return (AZValue *) ((char *) entry + impl->val_offset);
}

static inline void
clear_entry(AZHashMapImplementation *impl, AZHashMapEntry *entry)
{
    az_value_clear(impl->key_impl, key_ptr(impl, entry));
    az_value_clear(impl->val_impl, val_ptr(impl, entry));
}

static void
set_entry(AZHashMapImplementation *impl, AZHashMapEntry *entry, unsigned int next, const AZValue *key, const AZValue *val, unsigned int replace)
{
	entry->next = next;
	if (key) {
		AZValue *dst_key = key_ptr(impl, entry);
		if (replace) az_value_clear(impl->key_impl, dst_key);
        az_value_copy(impl->key_impl, dst_key, key);
	}
	AZValue *dst_val = val_ptr(impl, entry);
	if (replace) az_value_clear(impl->val_impl, dst_val);
    az_value_copy(impl->val_impl, dst_val, val);
}

static AZHashMapEntry *allocate_entries(AZHashMapImplementation *impl, unsigned int size, unsigned int root_size);
static void reallocate (AZHashMapImplementation *impl, AZHashMap *hmap, unsigned int new_root_size);

/* AZInterface implementation */
static void hmap_implementation_init (AZHashMapImplementation *impl);
static void hmap_instance_init (AZHashMapImplementation *impl, AZHashMap *hmap);
static void hmap_instance_finalize (AZHashMapImplementation *impl, AZHashMap *hmap);

static unsigned int hmap_type = 0;
static AZHashMapClass *hmap_class;

unsigned int
az_hash_map_get_type (void)
{
	if (!hmap_type) {
		hmap_class = (AZHashMapClass *) az_register_interface_type (&hmap_type, (const unsigned char *) "AZHashMap", AZ_TYPE_MAP,
			sizeof (AZMapClass), sizeof (AZMapImplementation), 0, AZ_FLAG_ZERO_MEMORY,
			NULL,
			(void (*) (AZImplementation *)) hmap_implementation_init,
            (void (*) (const AZImplementation *, void *)) hmap_instance_init,
            (void (*) (const AZImplementation *, void *)) hmap_instance_finalize);
	}
	return hmap_type;
}

static void
hmap_implementation_init (AZHashMapImplementation *impl)
{
    impl->key_impl = NULL;
    impl->val_impl = NULL;
    impl->root_size = 31;
    impl->entry_size = 0;
    impl->key_offset = 0;
    impl->key_size = 0;
    impl->val_offset = 0;
    impl->val_size = 0;
}

static void
hmap_instance_init (AZHashMapImplementation *impl, AZHashMap *hmap)
{
    hmap->root_size = impl->root_size;
    hmap->size = 3 * impl->root_size;
    hmap->free = hmap->root_size;
    hmap->entries = allocate_entries(impl, hmap->size, hmap->root_size);
}

static void
hmap_instance_finalize (AZHashMapImplementation *impl, AZHashMap *hmap)
{
	for (unsigned int i = 0; i < hmap->root_size; i++) {
		AZHashMapEntry *root_entry = entry_ptr(impl, hmap->entries, i);
		if (root_entry->next == EMPTY) continue;
		clear_entry(impl, root_entry);
		unsigned int pos = root_entry->next;
		while (pos != END) {
			AZHashMapEntry *entry = entry_ptr(impl, hmap->entries, pos);
			clear_entry(impl, entry);
			pos = entry->next;
		}
	}
    free(hmap->entries);
}

void
az_hash_map_insert(AZHashMapImplementation *impl, AZHashMap *hmap, const AZValue *key, const AZValue *val)
{
	hmap->n_entries += 1;
	unsigned int pos = impl->hash(impl, key) % hmap->root_size;
	AZHashMapEntry *root_entry = entry_ptr(impl, hmap->entries, pos);
	if(root_entry->next == EMPTY) {
		set_entry(impl, root_entry, END, key, val, 0);
		return;
	}
	if (impl->equal(impl, key, key_ptr(impl, root_entry))) {
		set_entry(impl, root_entry, root_entry->next, NULL, val, 1);
		return;
	}
	pos = root_entry->next;
	while (pos != END) {
		AZHashMapEntry *entry = entry_ptr(impl, hmap->entries, pos);
		if (impl->equal(impl, key, key_ptr(impl, entry))) {
			set_entry(impl, entry, entry->next, NULL, val, 1);
			return;
		}
		pos = entry->next;
	}
	if (hmap->free != END) {
		pos = hmap->free;
		AZHashMapEntry *entry = entry_ptr(impl, hmap->entries, pos);
		hmap->free = entry->next;
		set_entry(impl, entry, root_entry->next, key, val, 0);
		root_entry->next = pos;
		return;
	}
	reallocate (impl, hmap, hmap->root_size << 1);
	az_hash_map_insert(impl, hmap, key, val);
}

unsigned int
az_hash_map_remove(AZHashMapImplementation *impl, AZHashMap *hmap, const AZValue *key)
{
	unsigned int hval = impl->hash(impl, key) % hmap->root_size;
	AZHashMapEntry *root_entry = entry_ptr(impl, hmap->entries, hval);
	if(root_entry->next == EMPTY) return 0;
	if (impl->equal(impl, key, key_ptr(impl, root_entry))) {
		/* Have to remove root key */
		clear_entry(impl, root_entry);
		/* Move next key to root position */
		if (root_entry->next != END) {
			int pos = root_entry->next;
			AZHashMapEntry *entry = entry_ptr(impl, hmap->entries, pos);
			memcpy(root_entry, entry, impl->entry_size);
			/* Add old entry to the free list */
			entry->next = hmap->free;
			hmap->free = pos;
		} else {
			root_entry->next= EMPTY;
		}
		hmap->n_entries -= 1;
		return 1;
	}
	AZHashMapEntry *prev_entry = root_entry;
	int pos = root_entry->next;
	while (pos != END) {
		AZHashMapEntry *entry = entry_ptr(impl, hmap->entries, pos);
		if (impl->equal (impl, key, key_ptr(impl, entry))) {
			clear_entry(impl, entry);
			prev_entry->next = entry->next;
			entry->next = hmap->free;
			hmap->free = pos;
			hmap->n_entries -= 1;
			return 1;
		}
		prev_entry = entry;
		pos = entry->next;
	}
	return 0;
}

void
az_hash_map_clear(AZHashMapImplementation *impl, AZHashMap *hmap)
{
	for (unsigned int hval = 0; hval < hmap->root_size; hval++) {
		AZHashMapEntry *root_entry = entry_ptr(impl, hmap->entries, hval);
		if (root_entry->next == EMPTY) continue;
		clear_entry(impl, root_entry);
		unsigned int pos = root_entry->next;
		root_entry->next = EMPTY;
		while (pos != END) {
			AZHashMapEntry *entry = entry_ptr(impl, hmap->entries, pos);
			clear_entry(impl, entry);
			pos = entry->next;
		}
	}
	for (unsigned int pos = hmap->root_size; pos < (hmap->size - 1); pos++) {
		AZHashMapEntry *entry = entry_ptr(impl, hmap->entries, pos);
		entry->next = pos + 1;
	}
	AZHashMapEntry *entry = entry_ptr(impl, hmap->entries, hmap->size - 1);
	entry->next = END;
	hmap->free = hmap->root_size;
	hmap->n_entries = 0;
}

unsigned int
az_hash_map_exists(AZHashMapImplementation *impl, AZHashMap *hmap, const AZValue *key)
{
	return az_hash_map_lookup(impl, hmap, key) != NULL;
}

const AZValue *
az_hash_map_lookup(AZHashMapImplementation *impl, AZHashMap *hmap, const AZValue *key)
{
	unsigned int pos = impl->hash(impl, key) % hmap->root_size;
	AZHashMapEntry *entry = entry_ptr(impl, hmap->entries, pos);
	if (entry->next == EMPTY) return NULL;
	if (impl->equal (impl, key, key_ptr(impl, entry))) return val_ptr(impl, entry);
	for (pos = entry->next; pos != END; pos = entry->next) {
		entry = entry_ptr(impl, hmap->entries, pos);
		if (impl->equal (impl, key, key_ptr(impl, entry))) return val_ptr(impl, entry);
	}
	return NULL;
}

unsigned int
az_hash_map_forall (AZHashMapImplementation *impl, AZHashMap *hmap, unsigned int (* forall) (const AZValue *, const AZValue *, void *), void *data)
{
	for (unsigned int hval = 0; hval < hmap->root_size; hval++) {
		AZHashMapEntry *root_entry = entry_ptr(impl, hmap->entries, hval);
		if (root_entry->next == EMPTY) continue;
		if (!forall(key_ptr(impl, root_entry), val_ptr(impl, root_entry), data)) return 0;
		unsigned int pos = root_entry->next;
		while (pos != END) {
			AZHashMapEntry *entry = entry_ptr(impl, hmap->entries, pos);
			if (!forall(key_ptr(impl, entry), val_ptr(impl, entry), data)) return 0;
			pos = entry->next;
		}
	}
	return 1;
}

unsigned int
az_hash_map_remove_all (AZHashMapImplementation *impl, AZHashMap *hmap, unsigned int (*remove) (const AZValue *, const AZValue *, void *), void *data)
{
	unsigned int n_removed = 0;
	for (unsigned int hval = 0; hval < hmap->root_size; hval++) {
		AZHashMapEntry *root_entry = entry_ptr(impl, hmap->entries, hval);
		if (root_entry->next == EMPTY) continue;
		unsigned int pos = root_entry->next;
		AZHashMapEntry *prev;
		if (remove(key_ptr(impl, root_entry), val_ptr(impl, root_entry), data)) {
			/* Remove root entry */
			clear_entry(impl, root_entry);
			root_entry->next = EMPTY;
			hmap->n_entries -= 1;
			n_removed += 1;
			prev = NULL;
		} else {
			prev = root_entry;
		}
		while (pos != END) {
			AZHashMapEntry *entry = entry_ptr(impl, hmap->entries, pos);
			unsigned int next = entry->next;
			if (remove(key_ptr(impl, entry), val_ptr(impl, entry), data)) {
				/* Remove this entry */
				clear_entry(impl, entry);
				entry->next = hmap->free;
				hmap->free = pos;
				if (prev) prev->next = next;
				hmap->n_entries -= 1;
				n_removed += 1;
			} else {
				/* Keep this entry */
				if (!prev) {
					/* Move to root */
					memcpy(root_entry, entry, impl->entry_size);
					prev = root_entry;
					entry->next = hmap->free;
					hmap->free = pos;
				} else {
					prev = entry;
				}
			}
			pos = next;
		}
	}
	return n_removed;
}

static AZHashMapEntry *
allocate_entries(AZHashMapImplementation *impl, unsigned int size, unsigned int root_size)
{
	AZHashMapEntry *entries = aligned_alloc (16, size * impl->entry_size);
    for (unsigned int i = 0; i < root_size; i++) {
        *((unsigned int *) ((char *) entries + i * impl->entry_size)) = EMPTY;
    }
	for (unsigned int i = root_size; i < size - 1; i++) {
        *((unsigned int *) ((char *) entries + i * impl->entry_size)) = i + 1;
    }
    *((unsigned int *) ((char *) entries + (size - 1) * impl->entry_size)) = END;
	return entries;
}

static void
reallocate (AZHashMapImplementation *impl, AZHashMap *hmap, unsigned int new_root_size)
{
	unsigned int new_size = 3 * new_root_size;
	AZHashMapEntry *new_entries =  allocate_entries(impl, new_size, new_root_size);
	unsigned int new_free = new_root_size;
	for (unsigned int hval = 0; hval < hmap->root_size; hval++) {
		/* Skip if root is empty */
		AZHashMapEntry *root_entry = entry_ptr(impl, hmap->entries, hval);
		if(root_entry->next == EMPTY) continue;
		unsigned int pos = hval;
		do {
			AZHashMapEntry *entry = entry_ptr(impl, hmap->entries, pos);
			unsigned int new_hval = impl->hash(impl, key_ptr(impl, entry)) % new_root_size;
			AZHashMapEntry *new_root_entry = entry_ptr(impl, new_entries, new_hval);
			if (new_root_entry->next == EMPTY) {
				// Add new root
				memcpy(new_root_entry, entry, impl->entry_size);
				new_root_entry->next = END;
			} else {
				unsigned int new_next = new_root_entry->next;
				new_root_entry->next = new_free;
				AZHashMapEntry *new_entry = entry_ptr(impl, new_entries, new_free);
				new_free = new_entry->next;
				memcpy(new_entry, entry, impl->entry_size);
				new_entry->next = new_next;
			}
			pos = entry->next;
		} while (pos != END);
	}
	free (hmap->entries);
	hmap->root_size = new_root_size;
	hmap->size = new_size;
	hmap->free = new_free;
	hmap->entries = new_entries;
}
