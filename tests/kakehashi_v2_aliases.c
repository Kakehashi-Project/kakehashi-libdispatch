// Copyright 2026 Kakehashi Project
// SPDX-License-Identifier: Apache-2.0

// ABI delegation test: compile this with src/kakehashi_compat.c and dead-strip
// unrelated compatibility functions. It does not run the dispatch scheduler.
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

static void *seen_queue;
static const char *seen_label;
static void *seen_attr;
static void *seen_target;
static void *create_result;
static unsigned int assert_calls;
static unsigned int create_calls;

void dispatch_assert_queue_not(void *queue)
{
	seen_queue = queue;
	assert_calls++;
}

void *dispatch_queue_create_with_target(const char *label, void *attr, void *target)
{
	seen_label = label;
	seen_attr = attr;
	seen_target = target;
	create_calls++;
	return create_result;
}

extern void assert_not_v2(void *queue)
		__asm("_dispatch_assert_queue_not$V2");
extern void *create_target_v2(const char *label, void *attr, void *target)
		__asm("_dispatch_queue_create_with_target$V2");

int main(void)
{
	int queue, attr, target, result;
	const char label[] = "kh.alias.forwarding";

	assert_not_v2(&queue);
	assert(assert_calls == 1 && seen_queue == &queue);

	create_result = &result;
	assert(create_target_v2(label, &attr, &target) == &result);
	assert(create_calls == 1 && seen_label == label);
	assert(seen_attr == &attr && seen_target == &target);

	create_result = NULL;
	assert(create_target_v2(NULL, NULL, NULL) == NULL);
	assert(create_calls == 2 && seen_label == NULL);
	assert(seen_attr == NULL && seen_target == NULL);

	puts("dispatch V2 ABI delegation PASS");
	return 0;
}
