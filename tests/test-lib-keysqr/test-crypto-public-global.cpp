#include "gtest/gtest.h"
#include <string>
#include "lib-keysqr.hpp"

const std::string orderedKeySqrHrf =
	"A1tB2rC3bD4lE5tF6bG1tH1tI1tJ1tK1tL1tM1tN1tO1tP1tR1tS1tT1tU1tV1tW1tX1tY1tZ1t";
KeySqrFromString orderedTestKey = KeySqrFromString(orderedKeySqrHrf);
std::string defaultTestKeyDerivationOptionsJson = R"KGO({
	"purpose": "ForPublicKeySealedMesssagesWithRestrictionsEnforcedPostDecryption",
	"additionalSalt": "1"
})KGO";

TEST(PublicGlobal, DecryptionRestrictionsThowsOnInvalidJson) {
	ASSERT_ANY_THROW(
		MessageWithDecryptionRestrictions(SodiumBuffer(4, (unsigned char*)"test"), DecryptionRestrictions("badjson")).getPlaintext()
	);
}

TEST(PublicGlobal, DecryptionRestrictionsHandlesEmptyJson) {
	ASSERT_STREQ(
		(char*)MessageWithDecryptionRestrictions(
			SodiumBuffer(5, (unsigned char*)"test"),
			DecryptionRestrictions("{}")).getPlaintext().data,
		"test"
	);
}


TEST(PublicGlobal, DecryptionRestrictionsHandlesRestrictions) {
	DecryptionRestrictions dr(
		R"MYJSON(
			{
				"userMustAcknowledgeThisMessage": "yolo",
				"clientApplicationIdMustHavePrefix": ["myprefix"]
			}
		)MYJSON");
	const auto message = MessageWithDecryptionRestrictions(
		SodiumBuffer(5, (unsigned char*)"test"), dr);
	ASSERT_STREQ(message.getDecryptionRestrictions().userMustAcknowledgeThisMessage.c_str(), "yolo");
	ASSERT_STREQ(message.getDecryptionRestrictions().clientApplicationIdMustHavePrefix[0].c_str(), "myprefix");
	ASSERT_FALSE(dr.isApplicationIdAllowed("doesnotstartwithmyprefix"));
	ASSERT_TRUE(dr.isApplicationIdAllowed("myprefixisthestartofthisid"));
	ASSERT_TRUE(dr.isApplicationIdAllowed("myprefix"));
	ASSERT_STREQ( (char*) message.getPlaintext().data, "test");
}

TEST(PublicGlobal, GetsPublicKey) {
	const GlobalPublicPrivateKeyPair testGlobalPublicPrivateKeyPair(orderedTestKey, defaultTestKeyDerivationOptionsJson);
	const GlobalPublicKey testGlobalPublicKey = testGlobalPublicPrivateKeyPair.getPublicKey();

	ASSERT_EQ(testGlobalPublicKey.getPublicKeyBytesAsHexDigits().length(), 64);
}

TEST(PublicGlobal, ConvertsPublicKeyToJsonAndBack) {
	const GlobalPublicPrivateKeyPair testGlobalPublicPrivateKeyPair(orderedTestKey, defaultTestKeyDerivationOptionsJson);
	const GlobalPublicKey testGlobalPublicKey = testGlobalPublicPrivateKeyPair.getPublicKey();

	const std::string gpkJson = testGlobalPublicKey.toJson(1, '\t');
	const GlobalPublicKey gpk2(gpkJson);
	ASSERT_EQ(gpk2.getKeyDerivationOptionsJson(), defaultTestKeyDerivationOptionsJson);
	ASSERT_EQ(gpk2.getPublicKeyBytesAsHexDigits(), testGlobalPublicKey.getPublicKeyBytesAsHexDigits());
}

TEST(PublicGlobal, EncryptsAndDecrypts) {
	const GlobalPublicPrivateKeyPair testGlobalPublicPrivateKeyPair(orderedTestKey, defaultTestKeyDerivationOptionsJson);
	const GlobalPublicKey testGlobalPublicKey = testGlobalPublicPrivateKeyPair.getPublicKey();

	std::vector<unsigned char> messageVector = { 'y', 'o', 't', 'o' };
	SodiumBuffer messageBuffer(messageVector);
	MessageWithDecryptionRestrictions message(messageBuffer, DecryptionRestrictions("{}"));
	ASSERT_EQ(message.getDecryptionRestrictions().userMustAcknowledgeThisMessage, "");
	const auto sealedMessage = message.seal(testGlobalPublicKey);
	const auto unsealedMessage = testGlobalPublicPrivateKeyPair.unseal(sealedMessage);
	const auto unsealedPlaintext = unsealedMessage.getPlaintext().toVector();
	ASSERT_EQ(messageVector, unsealedPlaintext);
}
