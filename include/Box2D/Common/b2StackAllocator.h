/*
* Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef B2_STACK_ALLOCATOR_H
#define B2_STACK_ALLOCATOR_H

#include <Box2D/Common/b2Settings.h>

const int b2_stackSize = 100 * 1024;	// 100k
const int b2_maxStackEntries = 32;

struct b2StackEntry
{
	char* data;
	int size;
	bool usedMalloc;
};

// This is a stack allocator used for fast per step allocations.
// You must nest allocate/free pairs. The code will assert
// if you try to interleave multiple allocate/free pairs.
class b2StackAllocator
{
public:
	enum { MIN_ALIGNMENT = sizeof(void*) }; // Must be a power of 2
	enum { ALIGN_MASK = MIN_ALIGNMENT - 1 };

	b2StackAllocator();
	~b2StackAllocator();

	void* Allocate(int size);
	void* Reallocate(void* p, int size);
	void Free(void* p);

	int GetMaxAllocation() const;

private:

	char m_data[b2_stackSize];
	int m_index;

	int m_allocation;
	int m_maxAllocation;

	b2StackEntry m_entries[b2_maxStackEntries];
	int m_entryCount;
};

#endif
