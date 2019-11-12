#include "gtest/gtest.h"
#include "key-generation-options.hpp"


TEST(KeyGenerationOptions, GeneratesDefaults) {
	KeyGenerationOptions kgo = KeyGenerationOptions(R"KGO({
	"purpose": "ForApplicationUse"	
})KGO");
	ASSERT_EQ(
		kgo.jsonKeyGenerationOptionsWithAllOptionalParametersSpecified(1, '\t'),
		R"KGO({
	"hashFunction": "SHA256",
	"includeOrientationOfFacesInKey": false,
	"keyLengthInBytes": 32,
	"purpose": "ForApplicationUse"
})KGO"
	);
}

TEST(KeyGenerationOptions, InitsWithClientPrefixes) {
	KeyGenerationOptions kgo = KeyGenerationOptions(R"KGO({
	"purpose": "ForApplicationUse",
	"restictToClientApplicationsIdPrefixes": ["com.dicekeys.client", "com.dicekeys.another"]
})KGO");
	ASSERT_EQ(
		kgo.jsonKeyGenerationOptionsWithAllOptionalParametersSpecified(1, '\t'),
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
