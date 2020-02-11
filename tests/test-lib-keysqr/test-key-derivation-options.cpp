#include "gtest/gtest.h"
#include "lib-keysqr.hpp"


TEST(KeyDerivationOptions, GeneratesDefaults) {
	KeyDerivationOptions kgo = KeyDerivationOptions(R"KGO({
	"keyType": "Public"	
})KGO",
	KeyDerivationOptionsJson::KeyType::Public
);
	ASSERT_EQ(
		kgo.jsonKeyDerivationOptionsWithAllOptionalParametersSpecified(1, '\t'),
		R"KGO({
	"algorithm": "X25519",
	"hashFunction": "SHA256",
	"includeOrientationOfFacesInKey": false,
	"keyType": "Public"
})KGO"
	);
}


TEST(KeyDerivationOptions, FidoUseCase) {
	KeyDerivationOptions kgo = KeyDerivationOptions(R"KGO({
	"keyType": "Seed",
	"keyLengthInBytes": 96,
	"hashFunction": {"algorithm": "Argon2id"},
	"restictToClientApplicationsIdPrefixes": ["com.dicekeys.fido"]
})KGO",
	KeyDerivationOptionsJson::KeyType::Seed
);
	ASSERT_EQ(
		kgo.jsonKeyDerivationOptionsWithAllOptionalParametersSpecified(1, '\t'),
		R"KGO({
	"hashFunction": {
		"algorithm": "Argon2id",
		"memLimit": 67108864,
		"opsLimit": 2
	},
	"includeOrientationOfFacesInKey": false,
	"keyLengthInBytes": 96,
	"keyType": "Seed",
	"restictToClientApplicationsIdPrefixes": [
		"com.dicekeys.fido"
	]
})KGO"
);
}


TEST(KeyDerivationOptions, InitsWithClientPrefixes) {
	KeyDerivationOptions kgo = KeyDerivationOptions(R"KGO({
	"keyType": "Public",
	"restictToClientApplicationsIdPrefixes": ["com.dicekeys.client", "com.dicekeys.another"]
})KGO",
	KeyDerivationOptionsJson::KeyType::Public
);
	ASSERT_EQ(
		kgo.jsonKeyDerivationOptionsWithAllOptionalParametersSpecified(1, '\t'),
		R"KGO({
	"algorithm": "X25519",
	"hashFunction": "SHA256",
	"includeOrientationOfFacesInKey": false,
	"keyType": "Public",
	"restictToClientApplicationsIdPrefixes": [
		"com.dicekeys.client",
		"com.dicekeys.another"
	]
})KGO"
);
}
