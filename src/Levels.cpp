#include "Level.h"

#include "Level1.h"
#include "Level2.h"

TestEntry g_testEntries[] =
{
	{"Level 1", Level1::create},
	{"Level 2", Level2::create},
	{nullptr, nullptr}
};