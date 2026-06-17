#pragma once
#include "sqlite3.h"
#include <atomic>

void scheduler_loop(sqlite3* DB, std::atomic<bool>& running);