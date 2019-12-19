#include "gtest/gtest.h"
#include <string>
#include <iostream>
#include "lib-keysqr.hpp"

const std::string orderedKeySqrHrf =
	"A1tB2rC3bD4lE5tF6bG1tH1tI1tJ1tK1tL1tM1tN1tO1tP1tR1tS1tT1tU1tV1tW1tX1tY1tZ1t";
KeySqrFromString orderedTestKey = KeySqrFromString(orderedKeySqrHrf);
std::string defaultTestKeyDerivationOptionsJson = R"KGO({
	"purpose": "ForPublicKeySealedMessages",
	"additionalSalt": "1"
})KGO";

TEST(PublicGlobal, PostDecryptionInstructionsThowsOnInvalidJson) {
	ASSERT_ANY_THROW(
		Message(SodiumBuffer(4, (unsigned char*)"test"), "badjson").getPostDecryptionInstructions()
	);
}

TEST(PublicGlobal, PostDecryptionInstructionsHandlesEmptyJson) {
	ASSERT_STREQ(
		(char*)Message(
			SodiumBuffer(5, (unsigned char*)"test"),
			"{}").getPlaintext().data,
		"test"
	);
}

TEST(PublicGlobal, PostDecryptionInstructionsHandlesRestrictions) {
	std::string postDecryptionInstructionsJson =
		R"MYJSON(
			{
				"userMustAcknowledgeThisMessage": "yolo",
				"clientApplicationIdMustHavePrefix": ["myprefix"]
			}
		)MYJSON";
	const auto message = Message(
		SodiumBuffer(5, (unsigned char*)"test"), postDecryptionInstructionsJson);
	const auto dr = message.getPostDecryptionInstructions();
	ASSERT_STREQ(dr.userMustAcknowledgeThisMessage.c_str(), "yolo");
	ASSERT_STREQ(dr.clientApplicationIdMustHavePrefix[0].c_str(), "myprefix");
	ASSERT_FALSE(dr.isApplicationIdAllowed("doesnotstartwithmyprefix"));
	ASSERT_TRUE(dr.isApplicationIdAllowed("myprefixisthestartofthisid"));
	ASSERT_TRUE(dr.isApplicationIdAllowed("myprefix"));
	ASSERT_STREQ( (char*) message.getPlaintext().data, "test");
}

TEST(PublicGlobal, GetsPublicKey) {
	const PublicPrivateKeyPair testGlobalPublicPrivateKeyPair(orderedTestKey, defaultTestKeyDerivationOptionsJson);
	const PublicKey testGlobalPublicKey = testGlobalPublicPrivateKeyPair.getPublicKey();

	ASSERT_EQ(testGlobalPublicKey.getPublicKeyBytesAsHexDigits().length(), 64);
}

TEST(PublicGlobal, ConvertsPublicKeyToJsonAndBack) {
	const PublicPrivateKeyPair testGlobalPublicPrivateKeyPair(orderedTestKey, defaultTestKeyDerivationOptionsJson);
	const PublicKey testGlobalPublicKey = testGlobalPublicPrivateKeyPair.getPublicKey();

	const std::string gpkJson = testGlobalPublicKey.toJson(1, '\t');
	const PublicKey gpk2(gpkJson);
	ASSERT_EQ(gpk2.getKeyDerivationOptionsJson(), defaultTestKeyDerivationOptionsJson);
	ASSERT_EQ(gpk2.getPublicKeyBytesAsHexDigits(), testGlobalPublicKey.getPublicKeyBytesAsHexDigits());
}

TEST(PublicGlobal, EncryptsAndDecrypts) {
	const PublicPrivateKeyPair testGlobalPublicPrivateKeyPair(orderedTestKey, defaultTestKeyDerivationOptionsJson);
	const PublicKey testGlobalPublicKey = testGlobalPublicPrivateKeyPair.getPublicKey();

	const std::vector<unsigned char> messageVector = { 'y', 'o', 't', 'o' };
	const std::string postDecryptionInstructionsJson = "{}";
	SodiumBuffer messageBuffer(messageVector);
	Message message(messageBuffer, postDecryptionInstructionsJson);
	ASSERT_EQ(message.getPostDecryptionInstructions().userMustAcknowledgeThisMessage, "");
	const auto sealedMessage = message.seal(testGlobalPublicKey);
	const auto unsealedMessage = testGlobalPublicPrivateKeyPair.unseal(sealedMessage, postDecryptionInstructionsJson);
	const auto unsealedPlaintext = unsealedMessage.getPlaintext().toVector();
	ASSERT_EQ(messageVector, unsealedPlaintext);
}
