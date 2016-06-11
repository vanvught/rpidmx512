/* tables.h
 * Linker table helper macros.
 * NetWatch system management mode administration console
 *
 * Copyright 2009, Google Inc.
 * All rights reserved.
 * 
 * Copyright 2011, Jacob Potter.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,           
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY           
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TABLES_H
#define TABLES_H

/* Linker tables are a beautiful, evil piece of magic.
 *
 * The idea is this;
 * - For each table, define start and end pointers as zero-length arrays.
 * - Define however many data values as constants of the appropriate type
 * - Put each in its own section. For example, 'start' is in section
 *   .table.foo; the data elemnts are in section .table.foo.1 (or other values
 *   for explicit ordering); and 'end' is in .table.foo.END.
 * - Merge the sections together with the following linker script entry:
 *   *(SORT(.table.*))
 *
 * This has the effect that the start symbol points to the first value in the
 * table, and the end symbol points to the last. The number of values is
 * simply (table_foo_end - table_foo_start).
 */

typedef void (*thunk_t)(void);

typedef struct {
	thunk_t f;
	const char * name;
} initializer_t;

#ifndef TABLE_PREFIX
#define TABLE_PREFIX ".table."
#endif

#define TABLE_ITEMS(typ, sym, ...) const typ sym[] \
	__attribute__((section(TABLE_PREFIX #typ ".1"), aligned(4))) = { __VA_ARGS__ };

#define TABLE(typ, name) \
	const volatile typ name##_table[0] __attribute__((section(TABLE_PREFIX #name )));		\
	const volatile typ name##_table_end[0] __attribute__((section(TABLE_PREFIX #name ".END")));

#define INITIALIZER(table, f) const initializer_t f##_ptr \
	__attribute__((section(TABLE_PREFIX #table ".1." #f), aligned(4))) = { f, #f };

#define INITIALIZER_TABLE(name) \
	const volatile initializer_t name##_table[0] __attribute__((section(TABLE_PREFIX #name )));		\
	const volatile initializer_t name##_table_end[0] __attribute__((section(TABLE_PREFIX #name ".END")));

#define TABLE_LENGTH(name)	(name##_table_end - name##_table)

#endif /* TABLES_H */
