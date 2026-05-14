#define __AZ_HASH_SET_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2026
*/

#include <stdlib.h>
#include <string.h>

#include <az/base.h>
#include <az/value.h>
#include <az/packed-value.h>

#include "hash-set.h"

#ifdef _WIN32
#define aligned_alloc(a,s) _aligned_malloc(s,a)
#define aligned_free(p) _aligned_free(p)
#else
#define aligned_free(p) free(p)
#endif

struct _AZHashSetEntry {
    uint32_t next;
};

#define EMPTY 0
#define END 1

static AZHashSetEntry *
entry_ptr(const AZHashSetImplementation *impl, AZHashSetEntry *entries, unsigned int pos)
{
	return (AZHashSetEntry *) ((char *) entries + pos * impl->entry_size);
}

static AZValue *
elem_ptr(const AZHashSetImplementation *impl, AZHashSetEntry *entry)
{
	return (AZValue *) ((char *) entry + impl->elem_offset);
}

static void *
elem_inst(const AZHashSetImplementation *impl, AZHashSetEntry *entry)
{
	return az_value_get_inst(impl->elem_impl, elem_ptr(impl, entry));
}

static inline void
clear_entry(const AZHashSetImplementation *impl, AZHashSetEntry *entry)
{
    az_value_clear(impl->elem_impl, elem_ptr(impl, entry));
}

static void
set_entry(const AZHashSetImplementation *impl, AZHashSetEntry *entry, unsigned int next, void *elem)
{
	entry->next = next;
	AZValue *dst = elem_ptr(impl, entry);
    az_value_set_from_inst(impl->elem_impl, dst, elem);
}

static AZHashSetEntry *allocate_entries(const AZHashSetImplementation *impl, unsigned int size, unsigned int root_size);
static void reallocate (const AZHashSetImplementation *impl, AZHashSet *hset, unsigned int new_root_size);

static void hset_implementation_init (AZHashSetImplementation *impl);
static void hset_instance_init (const AZHashSetImplementation *impl, AZHashSet *hset);
static void hset_instance_finalize (const AZHashSetImplementation *impl, AZHashSet *hset);

static unsigned int hset_get_element_type (const AZCollectionImplementation *coll_impl, void *coll_inst);
static unsigned int hset_get_size (const AZCollectionImplementation *coll_impl, void *coll_inst);
static unsigned int hset_contains (const AZCollectionImplementation *coll_impl, void *coll_inst, const AZImplementation *impl, const void *inst);
static const AZImplementation *hset_get_iter (const AZCollectionImplementation *coll_impl, void *coll_inst, AZValue *iter);
static const AZImplementation *hset_iter_next (const AZCollectionImplementation *coll_impl, void *coll_inst, AZValue *iter);
static const AZImplementation *hset_get_element (const AZCollectionImplementation *coll_impl, void *coll_inst, const AZValue *iter, AZValue *val, unsigned int size);

static unsigned int hset_type = 0;
static AZHashSetClass *hset_class;

unsigned int
az_hash_set_get_type (void)
{
	if (!hset_type) {
		hset_class = (AZHashSetClass *) az_register_interface_type (&hset_type, (const unsigned char *) "AZHashSet", AZ_TYPE_SET,
			sizeof(AZHashSetClass), sizeof(AZHashSetImplementation), sizeof(AZHashSet), AZ_FLAG_ZERO_MEMORY | AZ_FLAG_CONSTRUCT,
			NULL,
			(void (*) (AZImplementation *)) hset_implementation_init,
            (void (*) (const AZImplementation *, void *)) hset_instance_init,
            (void (*) (const AZImplementation *, void *)) hset_instance_finalize);
	}
	return hset_type;
}

static void
hset_implementation_init (AZHashSetImplementation *impl)
{
	impl->set_impl.collection_impl.get_element_type = hset_get_element_type;
	impl->set_impl.collection_impl.get_size = hset_get_size;
	impl->set_impl.collection_impl.contains = hset_contains;
	impl->set_impl.collection_impl.get_iterator = hset_get_iter;
	impl->set_impl.collection_impl.iterator_next = hset_iter_next;
	impl->set_impl.collection_impl.get_element = hset_get_element;
    impl->elem_impl = NULL;
    impl->root_size = 31;
    impl->entry_size = 0;
    impl->elem_offset = 0;
    impl->elem_size = 0;
}

static void
hset_instance_init (const AZHashSetImplementation *impl, AZHashSet *hset)
{
    hset->root_size = impl->root_size;
    hset->size = 3 * impl->root_size;
    hset->free = hset->root_size;
    hset->entries = allocate_entries(impl, hset->size, hset->root_size);
}

static void
hset_instance_finalize (const AZHashSetImplementation *impl, AZHashSet *hset)
{
	for (unsigned int i = 0; i < hset->root_size; i++) {
		AZHashSetEntry *root_entry = entry_ptr(impl, hset->entries, i);
		if (root_entry->next == EMPTY) continue;
		clear_entry(impl, root_entry);
		unsigned int pos = root_entry->next;
		while (pos != END) {
			AZHashSetEntry *entry = entry_ptr(impl, hset->entries, pos);
			clear_entry(impl, entry);
			pos = entry->next;
		}
	}
    aligned_free(hset->entries);
}

static unsigned int
hset_get_element_type (const AZCollectionImplementation *coll_impl, void *coll_inst)
{
	AZHashSetImplementation *impl = (AZHashSetImplementation *) coll_impl;
	return AZ_IMPL_TYPE(impl->elem_impl);
}

static unsigned int
hset_get_size (const AZCollectionImplementation *coll_impl, void *coll_inst)
{
	AZHashSet *hset = (AZHashSet *) coll_inst;
	return hset->n_entries;
}

static unsigned int
hset_contains (const AZCollectionImplementation *coll_impl, void *coll_inst, const AZImplementation *impl, const void *inst)
{
	AZHashSetImplementation *hset_impl = (AZHashSetImplementation *) coll_impl;
	AZHashSet *hset = (AZHashSet *) coll_inst;
	return az_hash_set_contains(hset_impl, hset, inst);
}

static const AZImplementation *
hset_get_iter (const AZCollectionImplementation *coll_impl, void *coll_inst, AZValue *iter)
{
	AZHashSetImplementation *impl = (AZHashSetImplementation *) coll_impl;
	AZHashSet *hset = (AZHashSet *) coll_inst;
	for (unsigned int hval = 0; hval < hset->root_size; hval++) {
		AZHashSetEntry *root_entry = entry_ptr(impl, hset->entries, hval);
		if (root_entry->next != EMPTY) {
			iter->uint64_v = ((uint64_t) hval << 32) | hval;
			return &AZUint64Klass.impl;
		}
	}
	return NULL;
}

static const AZImplementation *
hset_iter_next (const AZCollectionImplementation *coll_impl, void *coll_inst, AZValue *iter)
{
	AZHashSetImplementation *impl = (AZHashSetImplementation *) coll_impl;
	AZHashSet *hset = (AZHashSet *) coll_inst;
	uint32_t hval = (uint32_t) (iter->uint64_v >> 32);
	uint32_t pos = (uint32_t) iter->uint64_v;
	AZHashSetEntry *entry = entry_ptr(impl, hset->entries, pos);
	if (entry->next != END) {
		iter->uint64_v = ((uint64_t) hval << 32) | entry->next;
		return &AZUint64Klass.impl;
	}
	for (hval += 1; hval < hset->root_size; hval++) {
		AZHashSetEntry *root_entry = entry_ptr(impl, hset->entries, hval);
		if (root_entry->next != EMPTY) {
			iter->uint64_v = ((uint64_t) hval << 32) | hval;
			return &AZUint64Klass.impl;
		}
	}
	return NULL;
}

static const AZImplementation *
hset_get_element (const AZCollectionImplementation *coll_impl, void *coll_inst, const AZValue *iter, AZValue *val, unsigned int size)
{
	AZHashSetImplementation *impl = (AZHashSetImplementation *) coll_impl;
	AZHashSet *hset = (AZHashSet *) coll_inst;
	uint32_t pos = (uint32_t) iter->uint64_v;
	AZHashSetEntry *entry = entry_ptr(impl, hset->entries, pos);
	return az_value_copy_autobox(impl->elem_impl, val, elem_ptr(impl, entry), size);
}

void
az_hash_set_insert(const AZHashSetImplementation *impl, AZHashSet *hset, void *elem)
{
	unsigned int pos = impl->hash(impl, elem) % hset->root_size;
	AZHashSetEntry *root_entry = entry_ptr(impl, hset->entries, pos);
	if(root_entry->next == EMPTY) {
		set_entry(impl, root_entry, END, elem);
		hset->n_entries += 1;
		return;
	}
	if (impl->equal(impl, elem, elem_inst(impl, root_entry))) {
		return;
	}
	pos = root_entry->next;
	while (pos != END) {
		AZHashSetEntry *entry = entry_ptr(impl, hset->entries, pos);
		if (impl->equal(impl, elem, elem_inst(impl, entry))) {
			return;
		}
		pos = entry->next;
	}
	if (hset->free != END) {
		pos = hset->free;
		AZHashSetEntry *entry = entry_ptr(impl, hset->entries, pos);
		hset->free = entry->next;
		set_entry(impl, entry, root_entry->next, elem);
		root_entry->next = pos;
		hset->n_entries += 1;
		return;
	}
	reallocate (impl, hset, hset->root_size << 1);
	az_hash_set_insert(impl, hset, elem);
}

unsigned int
az_hash_set_remove(const AZHashSetImplementation *impl, AZHashSet *hset, const void *elem)
{
	unsigned int hval = impl->hash(impl, elem) % hset->root_size;
	AZHashSetEntry *root_entry = entry_ptr(impl, hset->entries, hval);
	if(root_entry->next == EMPTY) return 0;
	if (impl->equal(impl, elem, elem_inst(impl, root_entry))) {
		clear_entry(impl, root_entry);
		if (root_entry->next != END) {
			int pos = root_entry->next;
			AZHashSetEntry *entry = entry_ptr(impl, hset->entries, pos);
			memcpy(root_entry, entry, impl->entry_size);
			entry->next = hset->free;
			hset->free = pos;
		} else {
			root_entry->next = EMPTY;
		}
		hset->n_entries -= 1;
		return 1;
	}
	AZHashSetEntry *prev_entry = root_entry;
	int pos = root_entry->next;
	while (pos != END) {
		AZHashSetEntry *entry = entry_ptr(impl, hset->entries, pos);
		if (impl->equal (impl, elem, elem_inst(impl, entry))) {
			clear_entry(impl, entry);
			prev_entry->next = entry->next;
			entry->next = hset->free;
			hset->free = pos;
			hset->n_entries -= 1;
			return 1;
		}
		prev_entry = entry;
		pos = entry->next;
	}
	return 0;
}

void
az_hash_set_clear(const AZHashSetImplementation *impl, AZHashSet *hset)
{
	for (unsigned int hval = 0; hval < hset->root_size; hval++) {
		AZHashSetEntry *root_entry = entry_ptr(impl, hset->entries, hval);
		if (root_entry->next == EMPTY) continue;
		clear_entry(impl, root_entry);
		unsigned int pos = root_entry->next;
		root_entry->next = EMPTY;
		while (pos != END) {
			AZHashSetEntry *entry = entry_ptr(impl, hset->entries, pos);
			clear_entry(impl, entry);
			pos = entry->next;
		}
	}
	for (unsigned int pos = hset->root_size; pos < (hset->size - 1); pos++) {
		AZHashSetEntry *entry = entry_ptr(impl, hset->entries, pos);
		entry->next = pos + 1;
	}
	AZHashSetEntry *entry = entry_ptr(impl, hset->entries, hset->size - 1);
	entry->next = END;
	hset->free = hset->root_size;
	hset->n_entries = 0;
}

unsigned int
az_hash_set_contains(const AZHashSetImplementation *impl, AZHashSet *hset, const void *elem)
{
	unsigned int pos = impl->hash(impl, elem) % hset->root_size;
	AZHashSetEntry *entry = entry_ptr(impl, hset->entries, pos);
	if (entry->next == EMPTY) return 0;
	if (impl->equal(impl, elem, elem_inst(impl, entry))) return 1;
	for (pos = entry->next; pos != END; pos = entry->next) {
		entry = entry_ptr(impl, hset->entries, pos);
		if (impl->equal(impl, elem, elem_inst(impl, entry))) return 1;
	}
	return 0;
}

unsigned int
az_hash_set_forall (const AZHashSetImplementation *impl, AZHashSet *hset, unsigned int (* forall) (const void *, void *), void *data)
{
	for (unsigned int hval = 0; hval < hset->root_size; hval++) {
		AZHashSetEntry *root_entry = entry_ptr(impl, hset->entries, hval);
		if (root_entry->next == EMPTY) continue;
		if (!forall(elem_inst(impl, root_entry), data)) return 0;
		unsigned int pos = root_entry->next;
		while (pos != END) {
			AZHashSetEntry *entry = entry_ptr(impl, hset->entries, pos);
			if (!forall(elem_inst(impl, entry), data)) return 0;
			pos = entry->next;
		}
	}
	return 1;
}

unsigned int
az_hash_set_remove_all (const AZHashSetImplementation *impl, AZHashSet *hset, unsigned int (*remove) (const void *, void *), void *data)
{
	unsigned int n_removed = 0;
	for (unsigned int hval = 0; hval < hset->root_size; hval++) {
		AZHashSetEntry *root_entry = entry_ptr(impl, hset->entries, hval);
		if (root_entry->next == EMPTY) continue;
		unsigned int pos = root_entry->next;
		AZHashSetEntry *prev;
		if (remove(elem_inst(impl, root_entry), data)) {
			clear_entry(impl, root_entry);
			root_entry->next = EMPTY;
			hset->n_entries -= 1;
			n_removed += 1;
			prev = NULL;
		} else {
			prev = root_entry;
		}
		while (pos != END) {
			AZHashSetEntry *entry = entry_ptr(impl, hset->entries, pos);
			unsigned int next = entry->next;
			if (remove(elem_inst(impl, entry), data)) {
				clear_entry(impl, entry);
				entry->next = hset->free;
				hset->free = pos;
				if (prev) prev->next = next;
				hset->n_entries -= 1;
				n_removed += 1;
			} else {
				if (!prev) {
					memcpy(root_entry, entry, impl->entry_size);
					prev = root_entry;
					entry->next = hset->free;
					hset->free = pos;
				} else {
					prev = entry;
				}
			}
			pos = next;
		}
	}
	return n_removed;
}

static AZHashSetEntry *
allocate_entries(const AZHashSetImplementation *impl, unsigned int size, unsigned int root_size)
{
	AZHashSetEntry *entries = (AZHashSetEntry *) aligned_alloc (16, size * impl->entry_size);
    memset(entries, 0, size * impl->entry_size);
    for (unsigned int i = root_size; i < size - 1; i++) {
        *((unsigned int *) ((char *) entries + i * impl->entry_size)) = i + 1;
    }
    *((unsigned int *) ((char *) entries + (size - 1) * impl->entry_size)) = END;
	return entries;
}

static void
reallocate (const AZHashSetImplementation *impl, AZHashSet *hset, unsigned int new_root_size)
{
	unsigned int new_size = 3 * new_root_size;
	AZHashSetEntry *new_entries =  allocate_entries(impl, new_size, new_root_size);
	unsigned int new_free = new_root_size;
	for (unsigned int hval = 0; hval < hset->root_size; hval++) {
		AZHashSetEntry *root_entry = entry_ptr(impl, hset->entries, hval);
		if(root_entry->next == EMPTY) continue;
		unsigned int pos = hval;
		do {
			AZHashSetEntry *entry = entry_ptr(impl, hset->entries, pos);
			unsigned int new_hval = impl->hash(impl, elem_inst(impl, entry)) % new_root_size;
			AZHashSetEntry *new_root_entry = entry_ptr(impl, new_entries, new_hval);
			if (new_root_entry->next == EMPTY) {
				memcpy(new_root_entry, entry, impl->entry_size);
				new_root_entry->next = END;
			} else {
				unsigned int new_next = new_root_entry->next;
				new_root_entry->next = new_free;
				AZHashSetEntry *new_entry = entry_ptr(impl, new_entries, new_free);
				new_free = new_entry->next;
				memcpy(new_entry, entry, impl->entry_size);
				new_entry->next = new_next;
			}
			pos = entry->next;
		} while (pos != END);
	}
	aligned_free (hset->entries);
	hset->root_size = new_root_size;
	hset->size = new_size;
	hset->free = new_free;
	hset->entries = new_entries;
}
