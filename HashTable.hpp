#pragma once

#include <cstddef>

//----------------------------------------------
//  hash_table
//
//  Maps pointer-sized integers to pointer-sized integers.
//  Uses open addressing with linear probing.
//  In the cells array, key = 0 is reserved to indicate an unused cell.
//  Actual value for key 0 (if any) is stored in cell_zero.
//  The hash table automatically doubles in size when it becomes 75% full.
//  The hash table never shrinks in size, even after clear(), unless you explicitly call compact().
//----------------------------------------------
namespace noob
{
	inline uint32_t upper_power_of_two(uint32_t v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
		return v;
	}

	inline uint64_t upper_power_of_two(uint64_t v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v |= v >> 32;
		v++;
		return v;
	}

	// from code.google.com/p/smhasher/wiki/MurmurHash3
	inline uint32_t integer_hash(uint32_t h)
	{
		h ^= h >> 16;
		h *= 0x85ebca6b;
		h ^= h >> 13;
		h *= 0xc2b2ae35;
		h ^= h >> 16;
		return h;
	}

	// from code.google.com/p/smhasher/wiki/MurmurHash3
	inline uint64_t integer_hash(uint64_t k)
	{
		k ^= k >> 33;
		k *= 0xff51afd7ed558ccd;
		k ^= k >> 33;
		k *= 0xc4ceb9fe1a85ec53;
		k ^= k >> 33;
		return k;
	}

	class hash_table
	{
		public:
			struct cell
			{
				size_t key;
				size_t value;
			};

		private:
			cell* cells;
			size_t array_size;
			size_t population;
			bool zero_used;
			cell cell_zero;

			void repopulate(size_t desiredSize);

		public:
			hash_table(size_t initialSize = 8);
			~hash_table();

			// Basic operations
			cell* lookup(size_t key);
			cell* insert(size_t key);
			void delete(cell* cell);
			void clear();
			void compact();

			void delete(size_t key)
			{
				cell* value = lookup(key);
				if (value)
				{
					delete(value);
				}
			}

			//----------------------------------------------
			//  iterator
			//----------------------------------------------
			friend class iterator;
			class iterator
			{
				private:
					hash_table& table;
					cell* current;

				public:
					iterator(hash_table &table);
					cell* next();
					inline cell* operator*() const { return current; }
					inline cell* operator->() const { return current; }
			};
	};
}
