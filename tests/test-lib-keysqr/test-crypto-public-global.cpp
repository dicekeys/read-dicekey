#include "gtest/gtest.h"
#include <string>
#include "lib-keysqr.hpp"

const std::string orderedKeySqrHrf =
	"A1tB2rC3bD4lE5tF6bG1tH1tI1tJ1tK1tL1tM1tN1tO1tP1tR1tS1tT1tU1tV1tW1tX1tY1tZ1t";
KeySqrFromString orderedTestKey = KeySqrFromString(orderedKeySqrHrf);

TEST(PublicGlobal, GetsPublicKey) {
	std::string keyDerivationOptionsJson = R"KGO({
	"purpose": "ForPublicKeySealedMesssagesWithRestrictionsEnforcedPostDecryption",
	"additionalSalt": "1"
})KGO";
	// KeyDerivationOptions kgo = KeyDerivationOptions(keyDerivationOptionsJson);
	const GlobalPublicPrivateKeyPair gsk(orderedTestKey, keyDerivationOptionsJson);
	const auto gpk = gsk.getPublicKey();
	ASSERT_EQ(gpk.)
}
