#include "gtest/gtest.h"
#include "key-generation-options.hpp";


TEST(KeyGenerationOptions, Inits) {
	KeyGenerationOptions kgo = KeyGenerationOptions(R"KGO({
	purpose: "ForApplicationUse"	
})KGO");
	ASSERT_EQ(
		kgo.jsonKeyGenerationOptionsWithAllOptionalParametersSpecified(0, '\t'),
		R"KGO({
	keyLengthInBytes: 32,
	purpose: "ForApplicationUse",
	restictToClientApplicationsIdPrefixes: []
})KGO"
	);
}

