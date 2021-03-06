﻿#include "stdafx.h"
#include "vector"
#include "list"
#include "intrin.h"
#include "iostream"
#include "string"
#include "Windows.h"

using namespace std;

struct Block
{
	size_t Level;
	bool Free;
	char* Data;
	Block* Next = nullptr;
	Block* Prev = nullptr;

	Block(size_t level, bool free, char* data)
		: Level(level), Free(free), Data(data) { }

	bool operator== (Block const& other) 
	{
		return Level == other.Level
			&& Free == other.Free
			&& Data == other.Data 
			&& Next == other.Next
			&& Prev == other.Prev;
	}
};

class Allocator
{
	size_t needed_size;
	size_t max_size;
	int list_count = get_list_count();
	char* memory = (char*)malloc(max_size);
	int mask = 0;
	vector<list<Block*>> block_lists = initialize_block_lists();
	
	string c1 = "Блоки размера";
	string c2 = "Начальная позиция блока";
	string c3 = "Занят";

	int get_list_count()
	{
		int i = 0;
		for (size_t size = needed_size - 1; size >= 1; size /= 2, i++);
		max_size = pow(2, i);
		return i;
	}

	vector<list<Block*>> initialize_block_lists()
	{
		vector<list<Block*>> result;
		result.reserve(list_count);
		for (int i = 0; i < list_count; i++)
		{
			list<Block*> list = {};
			result.push_back(list);
		}
		Block* block = new Block(list_count, true, memory);
		result[list_count - 1].push_back(block);
		set_byte_in_mask(block->Level);
		return result;
	}

	Block* split_block(Block* block)
	{
		size_t block_size = 1 << block->Level;
		Block* first_part = new Block(block->Level - 1, true, block->Data);
		Block* second_part = new Block(block->Level - 1, true, block->Data + block_size / 2);
		if (block->Prev)
			block->Prev->Next = first_part;
		first_part->Prev = block->Prev;
		first_part->Next = second_part;
		second_part->Prev = first_part;
		second_part->Next = block->Next;
		if (block->Next)
			block->Next->Prev = second_part;
		block_lists[first_part->Level - 1].push_back(first_part);
		block_lists[first_part->Level - 1].push_back(second_part);
		set_byte_in_mask(first_part->Level);
		block_lists[block->Level - 1].remove(block);
		if (block_lists[block->Level - 1].size() == 0)
			clear_byte_in_mask(block->Level);
		return first_part;
	}

	pair <Block*, int> join_blocks(Block* first_part, Block* second_part)
	{
		Block* block = new Block(first_part->Level + 1, true, first_part->Data);
		if (first_part->Prev)
			first_part->Prev->Next = block;
		block->Prev = first_part->Prev;
		block->Next = second_part->Next;
		if (second_part->Next)
			second_part->Next->Prev = block;
		block_lists[block->Level - 1].push_back(block);
		set_byte_in_mask(block->Level);
		block_lists[first_part->Level - 1].remove(first_part);
		block_lists[first_part->Level - 1].remove(second_part);
		if (block_lists[first_part->Level - 1].size() == 0)
			clear_byte_in_mask(first_part->Level);
		return { block, block_lists[block->Level - 1].size() - 1 };
	}

	void set_byte_in_mask(int position)
	{
		mask |= 1 << position;
	}

	void clear_byte_in_mask(int position)
	{
		mask &= ~(1 << position);
	}

	int get_index_of_desired_list(size_t desired_size)
	{
		unsigned long bsr_index;
		_BitScanReverse(&bsr_index, desired_size);
		auto M = (1 << (bsr_index + 1)) - 1;
		unsigned long bsf_index;
		_BitScanForward(&bsf_index, mask & ~M);
		return bsf_index - 1;
	}

	Block* get_first_free_block(int index)
	{
		for (int i = index; i < list_count; i++)
		{
			list<Block*> current_list = block_lists[i];
			for (Block* block : current_list)
			{
				if (block->Free)
					return block;
			}
		}
	}

	pair<Block*, int> get_block_with_data(void* ptr)
	{
		for (list<Block*> current_list : block_lists)
		{
			int index = 0;
			for (Block* current_block : current_list)
			{
				if (current_block->Data == ptr)
				{
					return { current_block, index };
				}
				index++;
			}
		}
	}

	void Free(Block* block_to_free, int index)
	{
		block_to_free->Free = true;
		if (index % 2 == 0)
		{
			auto next = block_to_free->Next;
			if (next)
			{
				if (next->Free && next->Level == block_to_free->Level)
				{
					auto block_and_index = join_blocks(block_to_free, next);
					Free(block_and_index.first, block_and_index.second);
				}
			}
		}
		else
		{
			auto prev = block_to_free->Prev;
			if (prev)
			{
				if (prev->Free && prev->Level == block_to_free->Level)
				{
					auto block_and_index = join_blocks(prev, block_to_free);
					Free(block_and_index.first, block_and_index.second);
				}
			}
		}		
	}

public:
	Allocator(size_t size) : needed_size(size) { }

	void* Alloc(size_t size)
	{
		int index_of_desired_list = get_index_of_desired_list(size);
		Block* current_block = get_first_free_block(index_of_desired_list);

		for (size_t current_size = 1 << current_block->Level; current_size >= 2; current_size /= 2)
		{
			if (size > current_size / 2)
			{
				current_block->Free = false;
				return current_block->Data;
			}
			else
			{
				current_block = split_block(current_block);
			}
		}
	}
	
	void Free(void *ptr)
	{
		pair<Block*, int> block_and_index = get_block_with_data(ptr);
		Block* block_to_free = block_and_index.first;
		int index = block_and_index.second;
		Free(block_to_free, index);
	}

	void Dump()
	{
		int current_size = 2;
		for (list<Block*> current_list : block_lists)
		{
			if (current_list.size() != 0)
			{
				printf("%s %d:\n", c1.c_str(), current_size);
				for (Block* current_block : current_list)
				{
					auto start_position = (int)(current_block->Data) % (int)memory;
					printf("\t%s = %d; %s = %d\n", c2.c_str(), start_position, c3.c_str(), !current_block->Free);
				}
				printf("\n");
			}
			current_size *= 2;
		}
	}
};

int main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	Allocator allocator = Allocator(1024);
	auto mem1 = allocator.Alloc(512);
	auto mem2 = allocator.Alloc(120);
	auto mem3 = allocator.Alloc(120);
	auto mem4 = allocator.Alloc(120);
	auto mem5 = allocator.Alloc(120);
	allocator.Free(mem3);
	allocator.Free(mem4);
	allocator.Free(mem2);
	allocator.Dump();
    return 0;
}
