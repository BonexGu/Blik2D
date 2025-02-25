#include <fstream>
#include <gtest/gtest.h>
#include BLIK_OPENH264_U_codec_def_h //original-code:"codec_def.h"
#include BLIK_OPENH264_U_codec_app_def_h //original-code:"codec_app_def.h"
#include "utils/BufferedData.h"
#include "BaseDecoderTest.h"

static void ReadFrame (std::ifstream* file, BufferedData* buf) {
  // start code of a frame is {0, 0, 0, 1}
  int zeroCount = 0;
  char b;

  buf->Clear();
  for (;;) {
    file->read (&b, 1);
    if (file->gcount() != 1) { // end of file
      return;
    }
    if (!buf->PushBack (b)) {
      FAIL() << "unable to allocate memory";
    }

    if (buf->Length() <= 4) {
      continue;
    }

    if (zeroCount < 3) {
      zeroCount = b != 0 ? 0 : zeroCount + 1;
    } else {
      if (b == 1) {
        if (file->seekg (-4, file->cur).good()) {
          buf->SetLength (buf->Length() - 4);
          return;
        } else {
          FAIL() << "unable to seek file";
        }
      } else if (b == 0) {
        zeroCount = 3;
      } else {
        zeroCount = 0;
      }
    }
  }
}

BaseDecoderTest::BaseDecoderTest()
  : decoder_ (NULL), decodeStatus_ (OpenFile) {}

int32_t BaseDecoderTest::SetUp() {
  long rv = WelsCreateDecoder (&decoder_);
  EXPECT_EQ (0, rv);
  EXPECT_TRUE (decoder_ != NULL);
  if (decoder_ == NULL) {
    return rv;
  }

  SDecodingParam decParam;
  memset (&decParam, 0, sizeof (SDecodingParam));
  decParam.uiTargetDqLayer = UCHAR_MAX;
  decParam.eEcActiveIdc = ERROR_CON_SLICE_COPY;
  decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;

  rv = decoder_->Initialize (&decParam);
  EXPECT_EQ (0, rv);
  return (int32_t)rv;
}

void BaseDecoderTest::TearDown() {
  if (decoder_ != NULL) {
    decoder_->Uninitialize();
    WelsDestroyDecoder (decoder_);
  }
}


void BaseDecoderTest::DecodeFrame (const uint8_t* src, size_t sliceSize, Callback* cbk) {
  uint8_t* data[3];
  SBufferInfo bufInfo;
  memset (data, 0, sizeof (data));
  memset (&bufInfo, 0, sizeof (SBufferInfo));

  DECODING_STATE rv = decoder_->DecodeFrame2 (src, (int) sliceSize, data, &bufInfo);
  ASSERT_TRUE (rv == dsErrorFree);

  if (bufInfo.iBufferStatus == 1 && cbk != NULL) {
    const Frame frame = {
      {
        // y plane
        data[0],
        bufInfo.UsrData.sSystemBuffer.iWidth,
        bufInfo.UsrData.sSystemBuffer.iHeight,
        bufInfo.UsrData.sSystemBuffer.iStride[0]
      },
      {
        // u plane
        data[1],
        bufInfo.UsrData.sSystemBuffer.iWidth / 2,
        bufInfo.UsrData.sSystemBuffer.iHeight / 2,
        bufInfo.UsrData.sSystemBuffer.iStride[1]
      },
      {
        // v plane
        data[2],
        bufInfo.UsrData.sSystemBuffer.iWidth / 2,
        bufInfo.UsrData.sSystemBuffer.iHeight / 2,
        bufInfo.UsrData.sSystemBuffer.iStride[1]
      },
    };
    cbk->onDecodeFrame (frame);
  }
}
void BaseDecoderTest::DecodeFile (const char* fileName, Callback* cbk) {
  std::ifstream file (fileName, std::ios::in | std::ios::binary);
  ASSERT_TRUE (file.is_open());

  BufferedData buf;
  while (true) {
    ReadFrame (&file, &buf);
    if (::testing::Test::HasFatalFailure()) {
      return;
    }
    if (buf.Length() == 0) {
      break;
    }
    DecodeFrame (buf.data(), buf.Length(), cbk);
    if (::testing::Test::HasFatalFailure()) {
      return;
    }
  }

  int32_t iEndOfStreamFlag = 1;
  decoder_->SetOption (DECODER_OPTION_END_OF_STREAM, &iEndOfStreamFlag);

  // Get pending last frame
  DecodeFrame (NULL, 0, cbk);
}

bool BaseDecoderTest::Open (const char* fileName) {
  if (decodeStatus_ == OpenFile) {
    file_.open (fileName, std::ios_base::out | std::ios_base::binary);
    if (file_.is_open()) {
      decodeStatus_ = Decoding;
      return true;
    }
  }
  return false;
}

bool BaseDecoderTest::DecodeNextFrame (Callback* cbk) {
  switch (decodeStatus_) {
  case Decoding:
    ReadFrame (&file_, &buf_);
    if (::testing::Test::HasFatalFailure()) {
      return false;
    }
    if (buf_.Length() == 0) {
      decodeStatus_ = EndOfStream;
      return true;
    }
    DecodeFrame (buf_.data(), buf_.Length(), cbk);
    if (::testing::Test::HasFatalFailure()) {
      return false;
    }
    return true;
  case EndOfStream: {
    int32_t iEndOfStreamFlag = 1;
    decoder_->SetOption (DECODER_OPTION_END_OF_STREAM, &iEndOfStreamFlag);
    DecodeFrame (NULL, 0, cbk);
    decodeStatus_ = End;
    break;
  }
  case OpenFile:
  case End:
    break;
  }
  return false;
}
