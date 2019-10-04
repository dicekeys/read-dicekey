

fun readKeySqrFromImageReaderJson(image: ImageReader) {
  val width: int = image.getWidth();
  val height: int = image.getHeight();
  val grayscalePlane: Image.Plane = image.getPlanes()[0];
  val bytesPerRow = grayscalePlane.getRowStride();
  val grayscalePlaneBuffer: ByteBuffer = grayscalePlane.getBuffer();
  return readKeySquare(width, height, bytesPerRow, planeByteArray);
}

external fun readKeySquareJson(
  width: Int,
  height: Int,
  bytesPerRow: Int,
  grayscalePlaneBuffer: ByteBuffer
): String
