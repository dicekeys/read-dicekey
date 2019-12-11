#include "gtest/gtest.h"
#include "lib-keysqr.hpp"


TEST(KeyDerivationOptions, GeneratesDefaults) {
	KeyDerivationOptions kgo = KeyDerivationOptions(R"KGO({
	"purpose": "ForApplicationUse"	
})KGO");
	ASSERT_EQ(
		kgo.jsonKeyDerivationOptionsWithAllOptionalParametersSpecified(1, '\t'),
		R"KGO({
	"hashFunction": "SHA256",
	"includeOrientationOfFacesInKey": false,
	"keyLengthInBytes": 32,
	"purpose": "ForApplicationUse"
})KGO"
	);
}

TEST(KeyDerivationOptions, InitsWithClientPrefixes) {
	KeyDerivationOptions kgo = KeyDerivationOptions(R"KGO({
	"purpose": "ForApplicationUse",
	"restictToClientApplicationsIdPrefixes": ["com.dicekeys.client", "com.dicekeys.another"]
})KGO");
	ASSERT_EQ(
		kgo.jsonKeyDerivationOptionsWithAllOptionalParametersSpecified(1, '\t'),
		R"KGO({
	"hashFunction": "SHA256",
	"includeOrientationOfFacesInKey": false,
	"keyLengthInBytes": 32,
	"purpose": "ForApplicationUse",
	"restictToClientApplicationsIdPrefixes": [
		"com.dicekeys.client",
		"com.dicekeys.another"
	]
})KGO"
);
}
