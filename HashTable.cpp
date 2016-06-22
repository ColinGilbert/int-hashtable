#include "HashTable.hpp"

#include <assert.h>
#include <memory.h>


#define FIRST_CELL(hash) (cells + ((hash) & (array_size - 1)))
#define CIRCULAR_NEXT(c) ((c) + 1 != cells + array_size ? (c) + 1 : cells)
#define CIRCULAR_OFFSET(a, b) ((b) >= (a) ? (b) - (a) : array_size + (b) - (a))


//----------------------------------------------
//  noob::hash_table::hash_table
//----------------------------------------------
noob::hash_table::hash_table(size_t initialSize)
{
	// Initialize regular cells
	array_size = initialSize;
	assert((array_size & (array_size - 1)) == 0);   // Must be a power of 2
	cells = new cell[array_size];
	memset(cells, 0, sizeof(cell) * array_size);
	population = 0;

	// Initialize zero cell
	zero_used = 0;
	cell_zero.key = 0;
	cell_zero.value = 0;
}

//----------------------------------------------
//  noob::hash_table::~hash_table
//----------------------------------------------
noob::hash_table::~hash_table()
{
	// delete regular cells
	delete[] cells;
}

//----------------------------------------------
//  noob::hash_table::lookup
//----------------------------------------------
noob::hash_table::cell* noob::hash_table::lookup(size_t key)
{
	if (key)
	{
		// Check regular cells
		for (cell* cell = FIRST_CELL(integer_hash(key));; cell = CIRCULAR_NEXT(cell))
		{
			if (cell->key == key)
				return cell;
			if (!cell->key)
				return NULL;
		}
	}
	else
	{
		// Check zero cell
		if (zero_used)
			return &cell_zero;
		return NULL;
	}
};

//----------------------------------------------
//  noob::hash_table::insert
//----------------------------------------------
noob::hash_table::cell* noob::hash_table::insert(size_t key)
{
	if (key)
	{
		// Check regular cells
		for (;;)
		{
			for (cell* cell = FIRST_CELL(integer_hash(key));; cell = CIRCULAR_NEXT(cell))
			{
				if (cell->key == key)
					return cell;        // Found
				if (cell->key == 0)
				{
					// insert here
					if ((population + 1) * 4 >= array_size * 3)
					{
						// Time to resize
						repopulate(array_size * 2);
						break;
					}
					++population;
					cell->key = key;
					return cell;
				}
			}
		}
	}
	else
	{
		// Check zero cell
		if (!zero_used)
		{
			// insert here
			zero_used = true;
			if (++population * 4 >= array_size * 3)
			{
				// Even though we didn't use a regular slot, let's keep the sizing rules consistent
				repopulate(array_size * 2);
			}
		}
		return &cell_zero;
	}
}

//----------------------------------------------
//  noob::hash_table::delete
//----------------------------------------------
void noob::hash_table::delete(cell* cell)
{
	if (cell != &cell_zero)
	{
		// delete from regular cells
		assert(cell >= cells && cell - cells < array_size);
		assert(cell->key);

		// Remove this cell by shuffling neighboring cells so there are no gaps in anyone's probe chain
		for (cell* neighbor = CIRCULAR_NEXT(cell);; neighbor = CIRCULAR_NEXT(neighbor))
		{
			if (!neighbor->key)
			{
				// There's nobody to swap with. Go ahead and clear this cell, then return
				cell->key = 0;
				cell->value = 0;
				population--;
				return;
			}
			cell* ideal = FIRST_CELL(integer_hash(neighbor->key));
			if (CIRCULAR_OFFSET(ideal, cell) < CIRCULAR_OFFSET(ideal, neighbor))
			{
				// Swap with neighbor, then make neighbor the new cell to remove.
				*cell = *neighbor;
				cell = neighbor;
			}
		}
	}
	else
	{
		// delete zero cell
		assert(zero_used);
		zero_used = false;
		cell->value = 0;
		population--;
		return;
	}
}

//----------------------------------------------
//  noob::hash_table::clear
//----------------------------------------------
void noob::hash_table::clear()
{
	// (Does not resize the array)
	// clear regular cells
	memset(cells, 0, sizeof(cell) * array_size);
	population = 0;
	// clear zero cell
	zero_used = false;
	cell_zero.value = 0;
}

//----------------------------------------------
//  noob::hash_table::compact
//----------------------------------------------
void noob::hash_table::compact()
{
	repopulate(upper_power_of_two((population * 4 + 3) / 3));
}

//----------------------------------------------
//  noob::hash_table::repopulate
//----------------------------------------------
void noob::hash_table::repopulate(size_t desiredSize)
{
	assert((desiredSize & (desiredSize - 1)) == 0);   // Must be a power of 2
	assert(population * 4  <= desiredSize * 3);

	// Get start/end pointers of old array
	cell* oldcells = cells;
	cell* end = cells + array_size;

	// Allocate new array
	array_size = desiredSize;
	cells = new cell[array_size];
	memset(cells, 0, sizeof(cell) * array_size);

	// Iterate through old array
	for (cell* c = oldcells; c != end; c++)
	{
		if (c->key)
		{
			// insert this element into new array
			for (cell* cell = FIRST_CELL(integer_hash(c->key));; cell = CIRCULAR_NEXT(cell))
			{
				if (!cell->key)
				{
					// insert here
					*cell = *c;
					break;
				}
			}
		}
	}

	// delete old array
	delete[] oldcells;
}

//----------------------------------------------
//  iterator::iterator
//----------------------------------------------
noob::hash_table::iterator::iterator(hash_table &table) : table(table)
{
	current = &table.cell_zero;
	if (!table.zero_used)
		next();
}

//----------------------------------------------
//  iterator::next
//----------------------------------------------
noob::hash_table::cell* noob::hash_table::iterator::next()
{
	// Already finished?
	if (!current)
		return current;

	// Iterate past zero cell
	if (current == &table.cell_zero)
		current = &table.cells[-1];

	// Iterate through the regular cells
	cell* end = table.cells + table.array_size;
	while (++current != end)
	{
		if (current->key)
			return current;
	}

	// Finished
	return current = NULL;
}
