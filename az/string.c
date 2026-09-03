#define __AZ_STRING_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <az/class.h>
#include <az/private.h>
#include <az/serialization.h>
#include <az/string.h>

#include "value.h"

#ifdef AZ_MT_REFERENCES
#include <arikkei/arikkei-threads.h>
#endif

/*
 * The collation table is guarded by a dedicated mutex.
 *
 * Lock ordering:
 * - registry -> dict: class constructors (holding the registry lock) create
 *   strings for property keys, so string creation never holds the dict mutex
 *   (instance init takes the registry lock)
 * - dict -> reference: a lookup holds the dict mutex across find + az_string_ref;
 *   the reference machinery never takes the dict mutex (the drop/dispose
 *   callbacks are invoked with no locks held)
 *
 * Resurrection safety: the last-reference drop callback (string_drop) verifies
 * the refcount under the dict mutex before removing the string from the table
 * and cancels disposal if a concurrent lookup resurrected it, so a lookup can
 * never return a string that is being freed.
 */
#ifdef AZ_MT_REFERENCES
static mtx_t dict_mutex;
#define AZ_STRING_LOCK() mtx_lock (&dict_mutex)
#define AZ_STRING_UNLOCK() mtx_unlock (&dict_mutex)
#else
#define AZ_STRING_LOCK()
#define AZ_STRING_UNLOCK()
#endif

typedef struct _AZStringLookup AZStringLookup;

struct _AZStringLookup {
	unsigned int len;
	const unsigned char *str;
};

static unsigned int
string_hash (const void *data)
{
	AZString *str = *((AZString **) data);
	return arikkei_memory_hash(str->str, str->length);
}

static unsigned int
string_equal (const void *l, const void *r)
{
	AZString *lhs = *((AZString **) l);
	AZString *rhs = *((AZString **) r);
	if (lhs->length != rhs->length) return 0;
	return !strcmp ((const char *) lhs->str, (const char *) rhs->str);
}

static unsigned int
string_data_hash (const void *data)
{
	AZStringLookup *lookup = (AZStringLookup *) data;
	return arikkei_memory_hash(lookup->str, lookup->len);
}

static unsigned int
string_data_equal (const void *l, const void *r)
{
	AZStringLookup *lhs = (AZStringLookup *) l;
	AZString *rhs = *((AZString **) r);
	if (lhs->len != rhs->length) return 0;
	return !strncmp ((const char *) lhs->str, (const char *) rhs->str, lhs->len);
}

static unsigned int
string_to_string (const AZImplementation *impl, void *instance, unsigned char *buf, unsigned int len)
{
	if (instance) {
		AZString *str = (AZString *) instance;
		if (buf) {
			unsigned int slen = (str->length > len) ? len : str->length;
			memcpy (buf, str->str, slen);
			if (str->length < len) buf[str->length] = 0;
		}
		return str->length;
	}
	if (buf && len) buf[0] = 0;
	return 0;
}

static unsigned int
serialize_string (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx)
{
	AZString *str = (AZString *) inst;
	if (!str) {
		static const unsigned char b[9] = { 0, 0, 0, 0, 0 };
		return az_serialize_block (d, dlen, b, 5);
	} else {
		if ((5 + str->length) <= dlen) {
			az_serialize_int (d, dlen, &str->length, 4);
			az_serialize_block (d + 4, dlen - 4, str->str, str->length + 1);
		}
		return 5 + str->length;
	}
}

static unsigned int
deserialize_string (const AZImplementation *impl, AZValue *value, const unsigned char *s, unsigned int slen, AZContext *ctx)
{
	AZString **str = &value->string;
	unsigned int len;
	if (slen < 5) {
		*str = NULL;
		return 0;
	}
	az_deserialize_int (&len, 4, s, slen);
	if ((5 + len) > slen) {
		*str = NULL;
		return 0;
	}
	*str = az_string_new_length (s + 4, len);
	return 5 + len;
}

/*
 * Remove the string from the collation table if it is the collated instance.
 * Pointer identity matters: a content-equal duplicate (e.g. the discarded result
 * of az_string_concat) is not a member and must not remove its collated twin
 * (the table looks members up by content).
 * The dict mutex is held by the caller.
 */
static void
string_uncollate (AZString *str)
{
	AZString **ptr = (AZString **) arikkei_dict_lookup (&AZStringKlass.chr2str, &str);
	if (ptr && (*ptr == str)) arikkei_dict_remove_pval (&AZStringKlass.chr2str, str);
}

/*
 * Called at the last reference with no locks held. Removes the string from the
 * collation table, unless it was resurrected by a concurrent lookup.
 */
static unsigned int
string_drop (AZReferenceClass *klass, AZReference *ref)
{
	AZString *str = (AZString *) ref;
	unsigned int die;
	AZ_STRING_LOCK();
	die = (str->reference.refcount == 1);
	if (die) string_uncollate (str);
	AZ_STRING_UNLOCK();
	return die;
}

static void
string_dispose (AZReferenceClass *klass, AZReference *ref)
{
	AZ_STRING_LOCK();
	string_uncollate ((AZString *) ref);
	AZ_STRING_UNLOCK();
}

AZ_CLASS_ALIGN AZStringClass AZStringKlass = {
	.reference_class = {
		.klass = {
			.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_CONSTRUCT | AZ_FLAG_REFERENCE | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_STRING },
			.parent = &AZReferenceKlass.klass,
			.name = (const uint8_t *) "string",
			.alignment = 7,
			.class_size = sizeof(AZStringClass),
			.instance_size = 0,
			.serialize = serialize_string,
			.deserialize = deserialize_string,
			.to_string = string_to_string
		},
		.drop = string_drop,
		.dispose = string_dispose
	},
	.chr2str = {0}
};

void
az_init_string_class (void)
{
#ifdef AZ_MT_REFERENCES
	mtx_init (&dict_mutex, mtx_plain);
#endif
	az_class_new_with_value(&AZStringKlass.reference_class.klass);
	arikkei_dict_setup_full (&AZStringKlass.chr2str, 701, string_hash, string_equal);
}

AZString *
az_string_new (const unsigned char *str)
{
	if (!str) return NULL;
	return az_string_new_length (str, (unsigned int) strlen ((const char *) str));
}

AZString *
az_string_new_length (const unsigned char *str, unsigned int length)
{
	/* Create outside the lock: instance init takes the registry lock, and the lock
	 * ordering is registry -> dict (class constructors create strings, not vice versa) */
	AZString *astr = (AZString *) malloc (sizeof (AZString) + length);
	az_instance_init_by_type (astr, AZ_TYPE_STRING);
	astr->length = length;
	memcpy ((unsigned char *) astr->str, str, length);
	((unsigned char *) astr->str)[length] = 0;
	AZStringLookup lookup = {length, str};
	AZ_STRING_LOCK();
	AZString **ptr = (AZString **) arikkei_dict_lookup_foreign(&AZStringKlass.chr2str, &lookup, string_data_hash(&lookup), string_data_equal);
	if (ptr) {
		/* Find + ref are atomic w.r.t. the last-reference removal (string_drop) */
		AZString *found = *ptr;
		az_string_ref (found);
		AZ_STRING_UNLOCK();
		az_string_unref (astr);
		return found;
	}
	arikkei_dict_insert_pval (&AZStringKlass.chr2str, astr, astr);
	AZ_STRING_UNLOCK();
	return astr;
}

AZString *
az_string_lookup (const unsigned char *chars)
{
	if (!chars) return NULL;
	return az_string_lookup_length (chars, (unsigned int) strlen ((const char *) chars));
}

AZString *
az_string_lookup_length (const unsigned char *chars, unsigned int length)
{
	AZStringLookup lookup = {length, chars};
	AZ_STRING_LOCK();
	AZString **ptr = (AZString **) arikkei_dict_lookup_foreign(&AZStringKlass.chr2str, &lookup, string_data_hash(&lookup), string_data_equal);
	if (!ptr) {
		AZ_STRING_UNLOCK();
		return NULL;
	}
	AZString *astr = *ptr;
	if (astr) az_string_ref (astr);
	AZ_STRING_UNLOCK();
	return astr;
}

AZString *
az_string_concat (AZString *lhs, AZString *rhs)
{
	AZString *built, *astr;
	if (!lhs) return rhs;
	if (!rhs) return lhs;
	built = (AZString *) malloc (sizeof (AZString) + lhs->length + rhs->length);
	az_instance_init_by_type (built, AZ_TYPE_STRING);
	built->length = lhs->length + rhs->length;
	if (lhs->length) memcpy ((unsigned char *) built->str, lhs->str, lhs->length);
	if (rhs->length) memcpy ((unsigned char *) built->str + lhs->length, rhs->str, rhs->length);
	((unsigned char *) built->str)[lhs->length + rhs->length] = 0;
	AZ_STRING_LOCK();
	AZString **ptr = (AZString **) arikkei_dict_lookup(&AZStringKlass.chr2str, &built);
	if (ptr) {
		/* The collated string gets a new reference, the duplicate is disposed */
		astr = *ptr;
		az_string_ref (astr);
		AZ_STRING_UNLOCK();
		az_string_unref (built);
		return astr;
	}
	arikkei_dict_insert_pval (&AZStringKlass.chr2str, built, built);
	AZ_STRING_UNLOCK();
	return built;
}

const unsigned char *
az_string_deserialize_chars (const unsigned char *cdata, unsigned int csize, unsigned int *cpos)
{
	unsigned int slen;
	const unsigned char *str;
	if ((csize - *cpos) < 5) return NULL;
	az_deserialize_int (&slen, 4, cdata + *cpos, csize - *cpos);
	if ((csize - *cpos) < (5 + slen)) return NULL;
	str = cdata + *cpos + 4;
	*cpos += (4 + slen + 1);
	return str;
}
