#ifndef TEST_H
#define TEST_H


#include "allocator.h"
#include "allocator_config.h"
#include "Stats.h"
#include <assert.h>
#include <unistd.h>
#include <stdio.h>wsl

void debug_log(const char *msg);
void basicTest();
void test_bigger_than_available_malloc();
void test_free();
void callTest(void (testFunction)(), const char* message);

#endif TEST_H

