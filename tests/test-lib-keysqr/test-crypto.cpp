#include "gtest/gtest.h"
#include <string>
#include <iostream>
#include "lib-keysqr.hpp"
#include "../lib-keysqr/derived-keys/convert.hpp"

//const std::string orderedKeySqrHrf =
//	"A1tB2rC3bD4lE5tF6bG1tH1tI1tJ1tK1tL1tM1tN1tO1tP1tR1tS1tT1tU1tV1tW1tX1tY1tZ1t";
// KeySqrFromString orderedTestKey = KeySqrFromString(orderedKeySqrHrf);
const std::string orderedTestKey = "A1tB2rC3bD4lE5tF6bG1tH1tI1tJ1tK1tL1tM1tN1tO1tP1tR1tS1tT1tU1tV1tW1tX1tY1tZ1t";
std::string defaultTestPublicKeyDerivationOptionsJson = R"KGO({
	"keyType": "Public",
	"additionalSalt": "1"
})KGO";
std::string defaultTestSymmetricKeyDerivationOptionsJson = R"KGO({
	"keyType": "Symmetric",
	"additionalSalt": "1"
})KGO";
std::string defaultTestSigningKeyDerivationOptionsJson = R"KGO({
	"keyType": "Signing",
	"additionalSalt": "1"
})KGO";


TEST(SeedGeneration, FidoUseCase) {
	std::string kdo = R"KDO({
	"keyType": "Seed",
	"keyLengthInBytes": 96,
	"hashFunction": {"algorithm": "Argon2id"},
	"restrictToClientApplicationsIdPrefixes": ["com.dicekeys.fido"]
})KDO";
	Seed seed(
		orderedTestKey,
		kdo,
		"com.dicekeys.fido"
	);
	const std::string seedAsHex = toHexStr(seed.reveal().toVector());
	ASSERT_EQ(
		seedAsHex,
		"83ef9982e73e98028397dca77b4d9bd92568af1c5b645896c88e6519a3abfd789d10b5c51df1b592a1bb205aceb579d9e07643f3da14e4c0fbafe9a485299a2b19d7bc33ebc20ea7025b5580dee2d7013239486fce04e97684ebf12dd70ed81e"
	);
}


TEST(PostDecryptionInstructions, ThowsOnInvalidJson) {
	ASSERT_ANY_THROW(
		PostDecryptionInstructions("badjson")
	);
}

TEST(PostDecryptionInstructions, Handles0LengthJsonObject) {
	ASSERT_STREQ(
		PostDecryptionInstructions("").userMustAcknowledgeThisMessage.c_str(),
		""
	);
}

TEST(PostDecryptionInstructions, HandlesEmptyJsonObject) {
	ASSERT_STREQ(
		PostDecryptionInstructions("{}").userMustAcknowledgeThisMessage.c_str(),
		""
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
	const auto dr = PostDecryptionInstructions(postDecryptionInstructionsJson);
	ASSERT_STREQ(dr.userMustAcknowledgeThisMessage.c_str(), "yolo");
	ASSERT_STREQ(dr.clientApplicationIdMustHavePrefix[0].c_str(), "myprefix");
	ASSERT_FALSE(dr.isApplicationIdAllowed("doesnotstartwithmyprefix"));
	ASSERT_TRUE(dr.isApplicationIdAllowed("myprefixisthestartofthisid"));
	ASSERT_TRUE(dr.isApplicationIdAllowed("myprefix"));
}

TEST(PublicKey, GetsPublicKey) {
	const PublicPrivateKeyPair testPublicPrivateKeyPair(orderedTestKey, defaultTestPublicKeyDerivationOptionsJson);
	const PublicKey testPublicKey = testPublicPrivateKeyPair.getPublicKey();

	ASSERT_EQ(testPublicKey.getPublicKeyBytesAsHexDigits().length(), 64);
}

TEST(PublicKey, GetsPublicKeyFromEmptyOptions) {
	const PublicPrivateKeyPair testPublicPrivateKeyPair(orderedTestKey, "{}");
	const PublicKey testPublicKey = testPublicPrivateKeyPair.getPublicKey();

	ASSERT_EQ(testPublicKey.getPublicKeyBytesAsHexDigits().length(), 64);
}

TEST(PublicKey, ConvertsToJsonAndBack) {
	const PublicPrivateKeyPair testPublicPrivateKeyPair(orderedTestKey, defaultTestPublicKeyDerivationOptionsJson);
	const PublicKey testPublicKey = testPublicPrivateKeyPair.getPublicKey();

	const std::string gpkJson = testPublicKey.toJson(1, '\t');
	const PublicKey gpk2(gpkJson);
	ASSERT_EQ(gpk2.getKeyDerivationOptionsJson(), defaultTestPublicKeyDerivationOptionsJson);
	ASSERT_EQ(gpk2.getPublicKeyBytesAsHexDigits(), testPublicKey.getPublicKeyBytesAsHexDigits());
}

TEST(PublicKey, EncryptsAndDecrypts) {
	const PublicPrivateKeyPair testPublicPrivateKeyPair(orderedTestKey, defaultTestPublicKeyDerivationOptionsJson);
	const PublicKey testPublicKey = testPublicPrivateKeyPair.getPublicKey();

	const std::vector<unsigned char> messageVector = { 'y', 'o', 't', 'o' };
	const std::string postDecryptionInstructionsJson = "{}";
	SodiumBuffer messageBuffer(messageVector);
	const auto sealedMessage = testPublicKey.seal(messageBuffer, postDecryptionInstructionsJson);
	const auto unsealedMessage = testPublicPrivateKeyPair.unseal(sealedMessage, postDecryptionInstructionsJson);
	const auto unsealedPlaintext = unsealedMessage.toVector();
	ASSERT_EQ(messageVector, unsealedPlaintext);
}


TEST(SigningKey, GetsSigningKey) {
	const SigningKey testSigningKey(orderedTestKey, defaultTestSigningKeyDerivationOptionsJson);
	const SignatureVerificationKey testSignatureVerificationKey = testSigningKey.getSignatureVerificationKey();

	ASSERT_EQ(testSignatureVerificationKey.getKeyBytesAsHexDigits().length(), 64);
}

TEST(SigningKey, GetsSigningKeyFromEmptyOptions) {
	const SigningKey testSigningKey(orderedTestKey, "{}");
	const SignatureVerificationKey testSignatureVerificationKey = testSigningKey.getSignatureVerificationKey();

	ASSERT_EQ(testSignatureVerificationKey.getKeyBytesAsHexDigits().length(), 64);
}

TEST(SigningKey, ConvertsToJsonAndBack) {
	const SigningKey testSigningKey(orderedTestKey, defaultTestSigningKeyDerivationOptionsJson);
	const SignatureVerificationKey testSignatureVerificationKey = testSigningKey.getSignatureVerificationKey();

	const std::string gpkJson = testSignatureVerificationKey.toJson(1, '\t');
	const SignatureVerificationKey gpk2(gpkJson);
	ASSERT_EQ(gpk2.getKeyDerivationOptionsJson(), defaultTestSigningKeyDerivationOptionsJson);
	ASSERT_STREQ(gpk2.getKeyBytesAsHexDigits().c_str(), testSignatureVerificationKey.getKeyBytesAsHexDigits().c_str());
}

TEST(SigningKey, Verification) {
	const SigningKey testSigningKey(orderedTestKey, defaultTestSigningKeyDerivationOptionsJson);
	const SignatureVerificationKey testSignatureVerificationKey = testSigningKey.getSignatureVerificationKey();

	const std::vector<unsigned char> messageVector = { 'y', 'o', 't', 'o' };
	const auto signature = testSigningKey.generateSignature(messageVector);
	const auto shouldVerifyAsTrue = testSignatureVerificationKey.verify(messageVector, signature);
	ASSERT_TRUE(shouldVerifyAsTrue);
	const std::vector<unsigned char> invalidMessageVector = { 'y', 'o', 'l', 'o' };
	const auto shouldVerifyAsFalse = testSignatureVerificationKey.verify(invalidMessageVector, signature);
	ASSERT_FALSE(shouldVerifyAsFalse);
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
	const std::vector<unsigned char> unsealedPlaintext = unsealedMessage.toVector();
	ASSERT_EQ(messageVector, unsealedPlaintext);
}


TEST(SymmetricKey, EncrypsUsingMessageAndDecrypts) {
	const SymmetricKey testSymmetricKey(orderedTestKey, defaultTestSymmetricKeyDerivationOptionsJson);
	
	const std::vector<unsigned char> messageVector = { 'y', 'o', 't', 'o' };
	const std::string postDecryptionInstructionsJson = "{\"userMustAcknowledgeThisMessage\": \"yoto mofo\"}";
	SodiumBuffer messageBuffer(messageVector);

	const auto sealedMessage = testSymmetricKey.seal(messageBuffer, postDecryptionInstructionsJson);
	const auto unsealedMessage = testSymmetricKey.unseal(sealedMessage, postDecryptionInstructionsJson);

	const std::vector<unsigned char> unsealedPlaintext = unsealedMessage.toVector();
	ASSERT_EQ(messageVector, unsealedPlaintext);
}

