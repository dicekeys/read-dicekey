#include "gtest/gtest.h"
#include "read-keysqr.hpp"
#include "validate-faces-read.h"
#include "visualize-read-results.h"
// for imread in tests files, imwrite if needed
#include <opencv2/imgcodecs.hpp>

void testFileWithObj(
  std::string filePath = std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".jpg"
) {
  cv::Mat bgraImage = cv::imread("tests/test-lib-read-keysqr/img/" + filePath, cv::IMREAD_COLOR);
  ASSERT_FALSE(bgraImage.empty()) << "No such file at " << filePath;
  cv::Mat rgbaImage;
  cv::cvtColor(bgraImage, rgbaImage, cv::COLOR_BGRA2RGBA);

  const size_t indexOfLastSlash = filePath.find_last_of("/") + 1;
  const std::string filename = filePath.substr(indexOfLastSlash);
  const std::string fileBase = filename.substr(0, filename.find_last_of("."));

  try {
    DiceKeyImageProcessor reader;
    reader.processRGBAImage(rgbaImage.cols, rgbaImage.rows, (uint32_t*) rgbaImage.data);
    auto keySqr = reader.keySqrRead();
    validateFacesRead(keySqr, filename.substr(0, 75));
  } catch (std::string errStr) {
    std::cerr << "Exception in " << filename << "\n  " << errStr << "\n";
    ASSERT_TRUE(false) << filename << "\n  " << errStr;
  }
}

TEST(KeySqrImageReaderFileTests, B21U11Z30O62W51C10D22T22F61X52I11R30L21H52A22K11P40J33V51Y41M33S20N63E60G32) {
  testFileWithObj("B21U11Z30O62W51C10D22T22F61X52I11R30L21H52A22K11P40J33V51Y41M33S20N63E60G32.jpg");
}

void testFile(
  std::string filePath = std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".jpg",
  bool validate = true,
  bool rotateToCanonicalForm = true,
  int maxErrorAllowed = 0,
	int facesExpectedToBeRead = NumberOfFaces
) {
  // #include <iostream>
  // #include <fstream>
	// std::ofstream myfile;
	// myfile.open("working-directory-is-here.txt");
	// myfile << "this is a stupid way to find the working directory";
	// myfile.close();

//	const char* basePathCStr = std::getenv("TEST_DIR");
//	const char* pathPathCStr = std::getenv("PATH");

//	const std::string basePath = basePathCStr ? std::string(basePathCStr) : "";
//  std::cerr << "Using base path" << basePath;
	//  const std::string currentPath = std::string(std::filesystem::current_path().u8string());
	// std::cerr << "In working directory" << currentPath;

	cv::Mat colorImage = cv::imread("tests/test-lib-read-keysqr/img/" + filePath, cv::IMREAD_COLOR);
  ASSERT_FALSE(colorImage.empty()) << "No such file at " << filePath;
	cv::Mat grayscaleImage, rgbaImage;
  cv::cvtColor(colorImage, grayscaleImage, cv::COLOR_BGR2GRAY);
  cv::cvtColor(colorImage, rgbaImage, cv::COLOR_BGR2RGBA);

  
  const size_t indexOfLastSlash = filePath.find_last_of("/") + 1;
  const std::string filename = filePath.substr(indexOfLastSlash);
  const std::string fileBase = filename.substr(0, filename.find_last_of("."));

  int totalError;
  try {
    const auto facesRead = readFaces(grayscaleImage, true);

    if (facesRead.faces.size() == 25) {
      const auto angleInRadiansNonCanonicalForm = facesRead.angleInRadiansNonCanonicalForm;
      const auto pixelsPerFaceEdgeWidth = facesRead.pixelsPerFaceEdgeWidth;
      const cv::Mat faceReadOutput = visualizeReadResults(rgbaImage, facesRead.faces, angleInRadiansNonCanonicalForm, pixelsPerFaceEdgeWidth);
      cv::Mat faceReadOutputBGR;
      cv::cvtColor(faceReadOutput, faceReadOutputBGR, cv::COLOR_RGBA2BGR);
      cv::imwrite("out/" + filename.substr(0, filename.length() - 4) + "-results.png", faceReadOutputBGR);
    }

    // Uncomment for debugging
    // cv::Mat colorImage;
    // cv::cvtColor(grayscaleImage, colorImage, cv::COLOR_GRAY2BGR);
    // for (auto const f : facesRead.faces) {
    //   cv::line(colorImage, f.underline.line.start, f.underline.line.end, cv::Scalar(255, 0, 255), 3);
    //   cv::line(colorImage, f.overline.line.start, f.overline.line.end, cv::Scalar(255, 255, 0), 3);
    // }
    // cv::imwrite("out/undoverlines/" + fileBase + ".png", colorImage);


    if (validate) {
      validateFacesRead(facesRead.faces, filename.substr(0, 75), rotateToCanonicalForm);
      // std::cerr << "Validated " << filename << "\n";
    }

		ASSERT_GE(facesExpectedToBeRead, facesRead.faces.size());

		if (facesRead.faces.size() >= NumberOfFaces) {
			const KeySqr<FaceRead> keySqrNonCanonical = KeySqr<FaceRead>(facesRead.faces);
			const KeySqr<FaceRead> keySqr = keySqrNonCanonical.rotateToCanonicalOrientation();
			totalError = keySqr.totalError();
		}
  } catch (std::string errStr) {
    std::cerr << "Exception in " << filename << "\n  " << errStr << "\n";
    ASSERT_TRUE(false) << filename << "\n  " << errStr;
  }
  ASSERT_LE(totalError, maxErrorAllowed);
}

TEST(DiceKeysTestInputs, D2tS2tP2lN2lO2bC2bA2lX1tG1lY2rH2lT2tR1lU2rM1tB2lV2lE2bZ1bF2tI1bJ2rL2lK2bW2t ) {
  testFile("D2tS2tP2lN2lO2bC2bA2lX1tG1lY2rH2lT2tR1lU2rM1tB2lV2lE2bZ1bF2tI1bJ2rL2lK2bW2t.jpg", true, true, 1);
}
TEST(DiceKeysTestInputs, A32W41T31I33Z52J21X20F21L63V52M43B33U22O43K30R21S62Y22P63E20G61H32N22C33D11 ) {
  testFile("A32W41T31I33Z52J21X20F21L63V52M43B33U22O43K30R21S62Y22P63E20G61H32N22C33D11.jpg", true, true, 1);
}
TEST(DiceKeysTestInputs, A53X23Y63Z60R32E53F31P33N42D21I62L21H11O11B10T40K63W40C50M12G12S50U61V51J40) {
	testFile("A53X23Y63Z60R32E53F31P33N42D21I62L21H11O11B10T40K63W40C50M12G12S50U61V51J40.jpg");
}
TEST(DiceKeysTestInputs, B21U11Z30O62W51C10D22T22F61X52I11R30L21H52A22K11P40J33V51Y41M33S20N63E60G32) {
  testFile("B21U11Z30O62W51C10D22T22F61X52I11R30L21H52A22K11P40J33V51Y41M33S20N63E60G32.jpg"); }
TEST(DiceKeysTestInputs, B23X21K21Y63F53I50O11H12J40M13G10P40C60S33U23A21W62L60V42D33T32Z61N13E33R63) {
  testFile("B23X21K21Y63F53I50O11H12J40M13G10P40C60S33U23A21W62L60V42D33T32Z61N13E33R63.jpg"); }
TEST(DiceKeysTestInputs, C22I12L11G51P31F53K22V40S13W53T31O50Z30B13M51E22J13H43U30A13D62N13R61X60Y41faded) {
  testFile("C22I12L11G51P31F53K22V40S13W53T31O50Z30B13M51E22J13H43U30A13D62N13R61X60Y41-faded.jpg", true, true, 1); }
TEST(DiceKeysTestInputs, C22I12L11G51P31F53K22V40S13W53T31O50Z30B13M51E22J13H43U30A13D62N13R61X60Y41) {
  testFile("C22I12L11G51P31F53K22V40S13W53T31O50Z30B13M51E22J13H43U30A13D62N13R61X60Y41-super-low-res.jpg", false, true, 6375); }
TEST(DiceKeysTestInputs, E12U31P11C51K13B31O53A60R51I21M12Y21T61Z60N10L61G40W21X12J30V31D22F33H21S32) {
  testFile("E12U31P11C51K13B31O53A60R51I21M12Y21T61Z60N10L61G40W21X12J30V31D22F33H21S32.jpg", true, true, 1); }
TEST(DiceKeysTestInputs, G21J20C42Y50L10Z12E40T23M62B20O10S12I11D22A30P32H31V33F52R43X21U62K41W40N32) {
  testFile("G21J20C42Y50L10Z12E40T23M62B20O10S12I11D22A30P32H31V33F52R43X21U62K41W40N32.jpg", false, true, 6375); }
TEST(DiceKeysTestInputs, H10A22Y23I43L42M60J20E61W32D43Z21V33T33G33K40X32B43N10R63O60U40S61F13C40P41) {
  testFile("H10A22Y23I43L42M60J20E61W32D43Z21V33T33G33K40X32B43N10R63O60U40S61F13C40P41.jpg"); }
TEST(DiceKeysTestInputs, H21Z40F20D13M20P20T50X33V11W51A43C51U31I12O63N42R33B12S51L42Y61G33J30E53K42angle) {
  testFile("H21Z40F20D13M20P20T50X33V11W51A43C51U31I12O63N42R33B12S51L42Y61G33J30E53K42-angle.jpg"); }
TEST(DiceKeysTestInputs, K13Y63A23S13X61C50P43M10B50R33L41V40D50G61U50I42E41T13H50F32O40N11W60J11Z31glare) {
  testFile("K13Y63A23S13X61C50P43M10B50R33L41V40D50G61U50I42E41T13H50F32O40N11W60J11Z31-glare.jpg"); }
TEST(DiceKeysTestInputs, K13Y63A23S13X61C50P43M10B50R33L41V40D50G61U50I42E41T13H50F32O40N11W60J11Z31) {
  testFile("K13Y63A23S13X61C50P43M10B50R33L41V40D50G61U50I42E41T13H50F32O40N11W60J11Z31.jpg"); }
TEST(DiceKeysTestInputs, R60D50Y32B60Z40O63L30K51J21M50G60P33X61E20A43U63S51F10C21H41V23N51T10I32W12) {
  testFile("R60D50Y32B60Z40O63L30K51J21M50G60P33X61E20A43U63S51F10C21H41V23N51T10I32W12.jpg", true, true, 4); }

TEST(DiceKeysTestInputs, A1bA6bA5bA4bA3bA2bA1bA6bA5bA4bA3bA2bA1bA6bA5bA4bA3bA2bA1bA6bA5bA4bA3bA2bA1b) {
  testFile("A1bA6bA5bA4bA3bA2bA1bA6bA5bA4bA3bA2bA1bA6bA5bA4bA3bA2bA1bA6bA5bA4bA3bA2bA1b.png", true, true, 0); }


TEST(DiceKeysTestInputs, OpenBox1_D2lC5rP1bK5bT1bY2bU1tG3rB6rZ5bS2tV5bO5bM4rJ4bX1tN6tA5rH1rW4lR4lE5bL1bI6rF6b) {
  testFile("D2lC5rP1bK5bT1bY2bU1tG3rB6rZ5bS2tV5bO5bM4rJ4bX1tN6tA5rH1rW4lR4lE5bL1bI6rF6b-openbox1.jpg", true, true, 0);
}
TEST(DiceKeysTestInputs, OpenBox2_D2lC5rP1bK5bT1bY2bU1tG3rB6rZ5bS2tV5bO5bM4rJ4bX1tN6tA5rH1rW4lR4lE5bL1bI6rF6b) {
  testFile("D2lC5rP1bK5bT1bY2bU1tG3rB6rZ5bS2tV5bO5bM4rJ4bX1tN6tA5rH1rW4lR4lE5bL1bI6rF6b-openbox2.jpg", true, true, 0);
}
TEST(DiceKeysTestInputs, OpenBox3_D2lC5rP1bK5bT1bY2bU1tG3rB6rZ5bS2tV5bO5bM4rJ4bX1tN6tA5rH1rW4lR4lE5bL1bI6rF6bdash1) {
  testFile("D2lC5rP1bK5bT1bY2bU1tG3rB6rZ5bS2tV5bO5bM4rJ4bX1tN6tA5rH1rW4lR4lE5bL1bI6rF6b-openbox3.jpg", true, true, 0);
}

TEST(DiceKeysTestInputs, S4tI3lZ2tR3lE2tW5bK3lD3rV3rF3tC6rG6rA3rU2tX4rO5tN6tL6lY4rJ4tM5lH6rP1tT2lB4t_well_lit) {
  testFile("S4tI3lZ2tR3lE2tW5bK3lD3rV3rF3tC6rG6rA3rU2tX4rO5tN6tL6lY4rJ4tM5lH6rP1tT2lB4t_well_lit.jpg", true, false, 0);
}

TEST(DiceKeysTestInputs, S4tI3lZ2tR3lE2tW5bK3lD3rV3rF3tC6rG6rA3rU2tX4rO5tN6tL6lY4rJ4tM5lH6rP1tT2lB4t) {
  testFile("S4tI3lZ2tR3lE2tW5bK3lD3rV3rF3tC6rG6rA3rU2tX4rO5tN6tL6lY4rJ4tM5lH6rP1tT2lB4t.jpg", true, false, 0);
}

/**
 * Tests we hope to pass with algorithmic improvements
 * 

TEST(DiceKeysTestInputs, U5bC4bE1lK4bD1lP6tW5bH6lN6tA4bJ3bM5rV6tL2tR1tT4tS5lI4lF3rY2tZ3tG1tX2bO1lB6r) {
  testFile("U5bC4bE1lK4bD1lP6tW5bH6lN6tA4bJ3bM5rV6tL2tR1tT4tS5lI4lF3rY2tZ3tG1tX2bO1lB6r.jpg", true, false, 0);
}

TEST(DiceKeysTestInputs, Y6bS2rG4bR4lF6lU5bH3tN4bX4lB2tP5tZ1lJ2bM1lE1lT1rL3lO1lI3tK6rW1tD3lV1lA6rC5b) {
  testFile("Y6bS2rG4bR4lF6lU5bH3tN4bX4lB2tP5tZ1lJ2bM1lE1lT1rL3lO1lI3tK6rW1tD3lV1lA6rC5b.jpg", true, false, 0);
}

*/
