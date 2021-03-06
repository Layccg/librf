﻿
#include <chrono>
#include <iostream>
#include <string>
#include <conio.h>

#include "librf.h"


static const intptr_t N = 10000000;
//static const int N = 10;

static std::mutex lock_console;

template <typename T>
void dump(size_t idx, std::string name, T start, T end)
{
	lock_console.lock();

	auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	std::cout << idx << ":" << name << "    ";
	std::cout << N << "      " << ns << " ns      ";
	std::cout << (ns / N) << " ns/op" << "    ";
	std::cout << (N * 100 * 1000 / ns) << "w/cps" << std::endl;

	lock_console.unlock();
}

auto yield_switch(intptr_t coro)
{
	for (intptr_t i = 0; i < N / coro; ++i)
		co_yield i;
	return N / coro;
}

void resumable_switch(intptr_t coro, size_t idx)
{
	resumef::local_scheduler ls;

	auto start = std::chrono::steady_clock::now();
	
	for (intptr_t i = 0; i < coro; ++i)
	{
		//go yield_switch(coro);
		go [=]
		{
			for (intptr_t i = 0; i < N / coro; ++i)
				co_yield i;
			return N / coro;
		};
	}
	resumef::this_scheduler()->run_until_notask();

	auto end = std::chrono::steady_clock::now();
	dump(idx, "BenchmarkSwitch_" + std::to_string(coro), start, end);
}

void resumable_main_resumable()
{
	//resumable_switch(1, 0);
	//resumable_switch(10, 0);
	//resumable_switch(100, 0);
	//resumable_switch(10000, 0);
	//resumable_switch(10000000, 0);

	std::thread works[32];
	for (size_t w = 1; w <= _countof(works); ++w)
	{
		for (size_t idx = 0; idx < w; ++idx)
			works[idx] = std::thread(&resumable_switch, 1000, idx);
		for (size_t idx = 0; idx < w; ++idx)
			works[idx].join();

		std::cout << std::endl << std::endl;
	}
}
