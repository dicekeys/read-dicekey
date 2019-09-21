#include "gtest/gtest.h"
#include "../../lib-dice-key/dice-key.h"


TEST(DiceKey, NotInitalized) {
	DiceKey key = DiceKey();
	ASSERT_EQ(key.initialized, false);
}
