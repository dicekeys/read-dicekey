#include "gtest/gtest.h"
#include <string>
#include <iostream>
#include "lib-keysqr.hpp"
#include "../lib-keysqr/derived-keys/convert.hpp"

const std::string orderedKeySqrHrf =
	"A1tB2rC3bD4lE5tF6bG1tH1tI1tJ1tK1tL1tM1tN1tO1tP1tR1tS1tT1tU1tV1tW1tX1tY1tZ1t";
KeySqrFromString orderedTestKey = KeySqrFromString(orderedKeySqrHrf);
std::string defaultTestKeyDerivationOptionsJson = R"KGO({
	"keyType": "Public",
	"additionalSalt": "1"
})KGO";
std::string defaultTestSymmetricKeyDerivationOptionsJson = R"KGO({
	"keyType": "Symmetric",
	"additionalSalt": "1"
})KGO";


TEST(SeedGeneration, FidoUseCase) {
	std::string kdo = R"KDO({
	"keyType": "Seed",
	"keyLengthInBytes": 96,
	"hashFunction": {"algorithm": "Argon2id"},
	"restictToClientApplicationsIdPrefixes": ["com.dicekeys.fido"]
})KDO";
	Seed seed(
		orderedTestKey,
		kdo,
		"com.dicekeys.fido"
	);
	const std::string seedAsHex = toHexStr(seed.reveal().toVector());
	ASSERT_EQ(
		seedAsHex,
		"b5c1555057994f34487b164d7ab84e3a097476bfe975fee8878b05357886e9b4495ab47ffa4aa89d30b33fcbff0b0209"
		"c924aeaf983251255d6e89946d466fd610c7c17a38897b0b5e3c9efec75be87600e2709b5537b6ec9b90c4f6b816f5af"
	);
}


TEST(PostDecryptionInstructions, ThowsOnInvalidJson) {
	ASSERT_ANY_THROW(
		Message(SodiumBuffer(4, (unsigned char*)"test"), "badjson").getPostDecryptionInstructions()
	);
}

TEST(PostDecryptionInstructions, Handles0LengthJsonObject) {
	ASSERT_STREQ(
		(char*)Message(
			SodiumBuffer(5, (unsigned char*)"test"),
			"").getPlaintext().data,
		"test"
	);
}

TEST(PostDecryptionInstructions, HandlesEmptyJsonObject) {
	ASSERT_STREQ(
		(char*)Message(
			SodiumBuffer(5, (unsigned char*)"test"),
			"{}").getPlaintext().data,
		"test"
	);
}

TEST(PostDecryptionInstructions, HandlesRestrictions) {
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

TEST(PublicKey, GetsPublicKey) {
	const PublicPrivateKeyPair testPublicPrivateKeyPair(orderedTestKey, defaultTestKeyDerivationOptionsJson);
	const PublicKey testPublicKey = testPublicPrivateKeyPair.getPublicKey();

	ASSERT_EQ(testPublicKey.getPublicKeyBytesAsHexDigits().length(), 64);
}

TEST(PublicKey, ConvertsyToJsonAndBack) {
	const PublicPrivateKeyPair testPublicPrivateKeyPair(orderedTestKey, defaultTestKeyDerivationOptionsJson);
	const PublicKey testPublicKey = testPublicPrivateKeyPair.getPublicKey();

	const std::string gpkJson = testPublicKey.toJson(1, '\t');
	const PublicKey gpk2(gpkJson);
	ASSERT_EQ(gpk2.getKeyDerivationOptionsJson(), defaultTestKeyDerivationOptionsJson);
	ASSERT_EQ(gpk2.getPublicKeyBytesAsHexDigits(), testPublicKey.getPublicKeyBytesAsHexDigits());
}

TEST(PublicKey, EncryptsAndDecrypts) {
	const PublicPrivateKeyPair testPublicPrivateKeyPair(orderedTestKey, defaultTestKeyDerivationOptionsJson);
	const PublicKey testPublicKey = testPublicPrivateKeyPair.getPublicKey();

	const std::vector<unsigned char> messageVector = { 'y', 'o', 't', 'o' };
	const std::string postDecryptionInstructionsJson = "{}";
	SodiumBuffer messageBuffer(messageVector);
	Message message(messageBuffer, postDecryptionInstructionsJson);
	ASSERT_EQ(message.getPostDecryptionInstructions().userMustAcknowledgeThisMessage, "");
	const auto sealedMessage = testPublicKey.seal(message);
	const auto unsealedMessage = testPublicPrivateKeyPair.unseal(sealedMessage, postDecryptionInstructionsJson);
	const auto unsealedPlaintext = unsealedMessage.getPlaintext().toVector();
	ASSERT_EQ(messageVector, unsealedPlaintext);
}


TEST(SymmetricKey, EncryptsAndDecryptsWithoutPostDecryptionInstructions) {
	const SymmetricKey testSymmetricKey(orderedTestKey, defaultTestSymmetricKeyDerivationOptionsJson);

	const std::vector<unsigned char> messageVector = { 'y', 'o', 't', 'o' };
	const std::string postDecryptionInstructionsJson = "";
	SodiumBuffer messageBuffer(messageVector);
	const auto sealedMessage = testSymmetricKey.seal(messageBuffer);
	const auto unsealedMessage = testSymmetricKey.unseal(sealedMessage);
	const auto unsealedPlaintext = unsealedMessage.toVector();
	ASSERT_EQ(messageVector, unsealedPlaintext);
}



TEST(SymmetricKey, EncryptsAndDecrypts) {
	const SymmetricKey testSymmetricKey(orderedTestKey, defaultTestSymmetricKeyDerivationOptionsJson);

	const std::vector<unsigned char> messageVector = { 'y', 'o', 't', 'o' };
	const std::string postDecryptionInstructionsJson = "{\"userMustAcknowledgeThisMessage\": \"yoto mofo\"}";
	SodiumBuffer messageBuffer(messageVector);
	
	const auto sealedMessage = testSymmetricKey.seal(messageBuffer, postDecryptionInstructionsJson);
	const auto unsealedMessage = testSymmetricKey.unseal(sealedMessage, postDecryptionInstructionsJson);
	ASSERT_EQ(unsealedMessage.getPostDecryptionInstructions().userMustAcknowledgeThisMessage, "yoto mofo");
	const auto unsealedPlaintext = unsealedMessage.getPlaintext().toVector();
	ASSERT_EQ(messageVector, unsealedPlaintext);
}


TEST(SymmetricKey, EncrypsUsingMessageAndDecrypts) {
	const SymmetricKey testSymmetricKey(orderedTestKey, defaultTestSymmetricKeyDerivationOptionsJson);
	
	const std::vector<unsigned char> messageVector = { 'y', 'o', 't', 'o' };
	const std::string postDecryptionInstructionsJson = "{\"userMustAcknowledgeThisMessage\": \"yoto mofo\"}";
	SodiumBuffer messageBuffer(messageVector);
	Message message(messageBuffer, postDecryptionInstructionsJson);
	ASSERT_EQ(message.getPostDecryptionInstructions().userMustAcknowledgeThisMessage, "yoto mofo");

	const auto sealedMessage = testSymmetricKey.seal(message);
	const auto unsealedMessage = testSymmetricKey.unseal(sealedMessage, postDecryptionInstructionsJson);
	ASSERT_EQ(unsealedMessage.getPostDecryptionInstructions().userMustAcknowledgeThisMessage, "yoto mofo");

	const auto unsealedPlaintext = unsealedMessage.getPlaintext().toVector();
	ASSERT_EQ(messageVector, unsealedPlaintext);
}

