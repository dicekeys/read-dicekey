#include "gtest/gtest.h"
#include "lib-keysqr.hpp"


TEST(KeyDerivationOptions, GeneratesDefaults) {
	KeyDerivationOptions kgo = KeyDerivationOptions(R"KGO({
	"keyType": "Public"	
})KGO",
	KeyDerivationOptionsJson::KeyType::Public
);
	ASSERT_EQ(
		kgo.keyDerivationOptionsJsonWithAllOptionalParametersSpecified(1, '\t'),
		R"KGO({
	"algorithm": "X25519",
	"hashFunction": "SHA256",
	"includeOrientationOfFacesInKey": true,
	"keyType": "Public"
})KGO"
	);
}


TEST(KeyDerivationOptions, FidoUseCase) {
	KeyDerivationOptions kgo = KeyDerivationOptions(R"KGO({
	"keyType": "Seed",
	"keyLengthInBytes": 96,
	"hashFunction": {"algorithm": "Argon2id"},
	"restrictToClientApplicationsIdPrefixes": ["com.dicekeys.fido"]
})KGO",
	KeyDerivationOptionsJson::KeyType::Seed
);
	ASSERT_EQ(
		kgo.keyDerivationOptionsJsonWithAllOptionalParametersSpecified(1, '\t'),
		R"KGO({
	"hashFunction": {
		"algorithm": "Argon2id",
		"memLimit": 67108864,
		"opsLimit": 2
	},
	"includeOrientationOfFacesInKey": true,
	"keyLengthInBytes": 96,
	"keyType": "Seed",
	"restrictToClientApplicationsIdPrefixes": [
		"com.dicekeys.fido"
	]
})KGO"
);
}


TEST(KeyDerivationOptions, InitsWithClientPrefixes) {
	KeyDerivationOptions kgo = KeyDerivationOptions(R"KGO({
	"keyType": "Public",
	"restrictToClientApplicationsIdPrefixes": ["com.dicekeys.client", "com.dicekeys.another"]
})KGO",
	KeyDerivationOptionsJson::KeyType::Public
);
	ASSERT_EQ(
		kgo.keyDerivationOptionsJsonWithAllOptionalParametersSpecified(1, '\t'),
		R"KGO({
	"algorithm": "X25519",
	"hashFunction": "SHA256",
	"includeOrientationOfFacesInKey": true,
	"keyType": "Public",
	"restrictToClientApplicationsIdPrefixes": [
		"com.dicekeys.client",
		"com.dicekeys.another"
	]
})KGO"
);
}
