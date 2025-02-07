--------------------------------------------------------------------
-- This file was automatically generated by ProjectGenerator
-- which is tooling part the build system designed for GUCEF
--     (Galaxy Unlimited Framework)
-- For the latest info, see http://www.VanvelzenSoftware.com/
--
-- The contents of this file are placed in the public domain. Feel
-- free to make use of it in any way you like.
--------------------------------------------------------------------
--

-- Configuration for module: OpenEXR


project( "OpenEXR" )

configuration( {} )
  location( os.getenv( "PM4OUTPUTDIR" ) )

configuration( {} )
  targetdir( os.getenv( "PM4TARGETDIR" ) )

configuration( {} )
language( "C++" )



configuration( {} )
kind( "SharedLib" )

configuration( {} )
links( { "zlib" } )
links( { "zlib" } )
  

configuration( { WIN32 } )
defines( { "HALF_EXPORTS", "IEX_EXPORTS", "ILMIMF_EXPORTS", "ILMTHREAD_EXPORTS", "IMATH_EXPORTS", "OPENEXR_DLL" } )
  

configuration( { WIN64 } )
defines( { "HALF_EXPORTS", "IEX_EXPORTS", "ILMIMF_EXPORTS", "ILMTHREAD_EXPORTS", "IMATH_EXPORTS", "OPENEXR_DLL" } )


configuration( {} )
vpaths { ["Headers"] = { "**.h", "**.hpp", "**.hxx" } }
files( {
  "IlmBase/Half/eLut.h",
  "IlmBase/Half/half.h",
  "IlmBase/Half/halfExport.h",
  "IlmBase/Half/halfFunction.h",
  "IlmBase/Half/halfLimits.h",
  "IlmBase/Half/toFloat.h",
  "IlmBase/Iex/Iex.h",
  "IlmBase/Iex/IexBaseExc.h",
  "IlmBase/Iex/IexErrnoExc.h",
  "IlmBase/Iex/IexExport.h",
  "IlmBase/Iex/IexForward.h",
  "IlmBase/Iex/IexMacros.h",
  "IlmBase/Iex/IexMathExc.h",
  "IlmBase/Iex/IexNamespace.h",
  "IlmBase/Iex/IexThrowErrnoExc.h",
  "IlmBase/IexMath/IexMathFloatExc.h",
  "IlmBase/IexMath/IexMathFpu.h",
  "IlmBase/IexMath/IexMathIeeeExc.h",
  "IlmBase/IlmThread/IlmThread.h",
  "IlmBase/IlmThread/IlmThreadExport.h",
  "IlmBase/IlmThread/IlmThreadForward.h",
  "IlmBase/IlmThread/IlmThreadMutex.h",
  "IlmBase/IlmThread/IlmThreadNamespace.h",
  "IlmBase/IlmThread/IlmThreadPool.h",
  "IlmBase/IlmThread/IlmThreadSemaphore.h",
  "IlmBase/Imath/ImathBox.h",
  "IlmBase/Imath/ImathBoxAlgo.h",
  "IlmBase/Imath/ImathColor.h",
  "IlmBase/Imath/ImathColorAlgo.h",
  "IlmBase/Imath/ImathEuler.h",
  "IlmBase/Imath/ImathExc.h",
  "IlmBase/Imath/ImathExport.h",
  "IlmBase/Imath/ImathForward.h",
  "IlmBase/Imath/ImathFrame.h",
  "IlmBase/Imath/ImathFrustum.h",
  "IlmBase/Imath/ImathFrustumTest.h",
  "IlmBase/Imath/ImathFun.h",
  "IlmBase/Imath/ImathGL.h",
  "IlmBase/Imath/ImathGLU.h",
  "IlmBase/Imath/ImathHalfLimits.h",
  "IlmBase/Imath/ImathInt64.h",
  "IlmBase/Imath/ImathInterval.h",
  "IlmBase/Imath/ImathLimits.h",
  "IlmBase/Imath/ImathLine.h",
  "IlmBase/Imath/ImathLineAlgo.h",
  "IlmBase/Imath/ImathMath.h",
  "IlmBase/Imath/ImathMatrix.h",
  "IlmBase/Imath/ImathMatrixAlgo.h",
  "IlmBase/Imath/ImathNamespace.h",
  "IlmBase/Imath/ImathPlane.h",
  "IlmBase/Imath/ImathPlatform.h",
  "IlmBase/Imath/ImathQuat.h",
  "IlmBase/Imath/ImathRandom.h",
  "IlmBase/Imath/ImathRoots.h",
  "IlmBase/Imath/ImathShear.h",
  "IlmBase/Imath/ImathSphere.h",
  "IlmBase/Imath/ImathVec.h",
  "IlmBase/Imath/ImathVecAlgo.h",
  "IlmBase/config.windows/IlmBaseConfig.h",
  "OpenEXR/IlmImf/ImfAcesFile.h",
  "OpenEXR/IlmImf/ImfArray.h",
  "OpenEXR/IlmImf/ImfAttribute.h",
  "OpenEXR/IlmImf/ImfAutoArray.h",
  "OpenEXR/IlmImf/ImfB44Compressor.h",
  "OpenEXR/IlmImf/ImfBoxAttribute.h",
  "OpenEXR/IlmImf/ImfCRgbaFile.h",
  "OpenEXR/IlmImf/ImfChannelList.h",
  "OpenEXR/IlmImf/ImfChannelListAttribute.h",
  "OpenEXR/IlmImf/ImfCheckedArithmetic.h",
  "OpenEXR/IlmImf/ImfChromaticities.h",
  "OpenEXR/IlmImf/ImfChromaticitiesAttribute.h",
  "OpenEXR/IlmImf/ImfCompositeDeepScanLine.h",
  "OpenEXR/IlmImf/ImfCompression.h",
  "OpenEXR/IlmImf/ImfCompressionAttribute.h",
  "OpenEXR/IlmImf/ImfCompressor.h",
  "OpenEXR/IlmImf/ImfConvert.h",
  "OpenEXR/IlmImf/ImfDeepCompositing.h",
  "OpenEXR/IlmImf/ImfDeepFrameBuffer.h",
  "OpenEXR/IlmImf/ImfDeepImageState.h",
  "OpenEXR/IlmImf/ImfDeepImageStateAttribute.h",
  "OpenEXR/IlmImf/ImfDeepScanLineInputFile.h",
  "OpenEXR/IlmImf/ImfDeepScanLineInputPart.h",
  "OpenEXR/IlmImf/ImfDeepScanLineOutputFile.h",
  "OpenEXR/IlmImf/ImfDeepScanLineOutputPart.h",
  "OpenEXR/IlmImf/ImfDeepTiledInputFile.h",
  "OpenEXR/IlmImf/ImfDeepTiledInputPart.h",
  "OpenEXR/IlmImf/ImfDeepTiledOutputFile.h",
  "OpenEXR/IlmImf/ImfDeepTiledOutputPart.h",
  "OpenEXR/IlmImf/ImfDoubleAttribute.h",
  "OpenEXR/IlmImf/ImfDwaCompressor.h",
  "OpenEXR/IlmImf/ImfDwaCompressorSimd.h",
  "OpenEXR/IlmImf/ImfEnvmap.h",
  "OpenEXR/IlmImf/ImfEnvmapAttribute.h",
  "OpenEXR/IlmImf/ImfExport.h",
  "OpenEXR/IlmImf/ImfFastHuf.h",
  "OpenEXR/IlmImf/ImfFloatAttribute.h",
  "OpenEXR/IlmImf/ImfFloatVectorAttribute.h",
  "OpenEXR/IlmImf/ImfForward.h",
  "OpenEXR/IlmImf/ImfFrameBuffer.h",
  "OpenEXR/IlmImf/ImfFramesPerSecond.h",
  "OpenEXR/IlmImf/ImfGenericInputFile.h",
  "OpenEXR/IlmImf/ImfGenericOutputFile.h",
  "OpenEXR/IlmImf/ImfHeader.h",
  "OpenEXR/IlmImf/ImfHuf.h",
  "OpenEXR/IlmImf/ImfIO.h",
  "OpenEXR/IlmImf/ImfInputFile.h",
  "OpenEXR/IlmImf/ImfInputPart.h",
  "OpenEXR/IlmImf/ImfInputPartData.h",
  "OpenEXR/IlmImf/ImfInputStreamMutex.h",
  "OpenEXR/IlmImf/ImfInt64.h",
  "OpenEXR/IlmImf/ImfIntAttribute.h",
  "OpenEXR/IlmImf/ImfKeyCode.h",
  "OpenEXR/IlmImf/ImfKeyCodeAttribute.h",
  "OpenEXR/IlmImf/ImfLineOrder.h",
  "OpenEXR/IlmImf/ImfLineOrderAttribute.h",
  "OpenEXR/IlmImf/ImfLut.h",
  "OpenEXR/IlmImf/ImfMatrixAttribute.h",
  "OpenEXR/IlmImf/ImfMisc.h",
  "OpenEXR/IlmImf/ImfMultiPartInputFile.h",
  "OpenEXR/IlmImf/ImfMultiPartOutputFile.h",
  "OpenEXR/IlmImf/ImfMultiView.h",
  "OpenEXR/IlmImf/ImfName.h",
  "OpenEXR/IlmImf/ImfNamespace.h",
  "OpenEXR/IlmImf/ImfOpaqueAttribute.h",
  "OpenEXR/IlmImf/ImfOptimizedPixelReading.h",
  "OpenEXR/IlmImf/ImfOutputFile.h",
  "OpenEXR/IlmImf/ImfOutputPart.h",
  "OpenEXR/IlmImf/ImfOutputPartData.h",
  "OpenEXR/IlmImf/ImfOutputStreamMutex.h",
  "OpenEXR/IlmImf/ImfPartHelper.h",
  "OpenEXR/IlmImf/ImfPartType.h",
  "OpenEXR/IlmImf/ImfPixelType.h",
  "OpenEXR/IlmImf/ImfPizCompressor.h",
  "OpenEXR/IlmImf/ImfPreviewImage.h",
  "OpenEXR/IlmImf/ImfPreviewImageAttribute.h",
  "OpenEXR/IlmImf/ImfPxr24Compressor.h",
  "OpenEXR/IlmImf/ImfRational.h",
  "OpenEXR/IlmImf/ImfRationalAttribute.h",
  "OpenEXR/IlmImf/ImfRgba.h",
  "OpenEXR/IlmImf/ImfRgbaFile.h",
  "OpenEXR/IlmImf/ImfRgbaYca.h",
  "OpenEXR/IlmImf/ImfRle.h",
  "OpenEXR/IlmImf/ImfRleCompressor.h",
  "OpenEXR/IlmImf/ImfScanLineInputFile.h",
  "OpenEXR/IlmImf/ImfSimd.h",
  "OpenEXR/IlmImf/ImfStandardAttributes.h",
  "OpenEXR/IlmImf/ImfStdIO.h",
  "OpenEXR/IlmImf/ImfStringAttribute.h",
  "OpenEXR/IlmImf/ImfStringVectorAttribute.h",
  "OpenEXR/IlmImf/ImfSystemSpecific.h",
  "OpenEXR/IlmImf/ImfTestFile.h",
  "OpenEXR/IlmImf/ImfThreading.h",
  "OpenEXR/IlmImf/ImfTileDescription.h",
  "OpenEXR/IlmImf/ImfTileDescriptionAttribute.h",
  "OpenEXR/IlmImf/ImfTileOffsets.h",
  "OpenEXR/IlmImf/ImfTiledInputFile.h",
  "OpenEXR/IlmImf/ImfTiledInputPart.h",
  "OpenEXR/IlmImf/ImfTiledMisc.h",
  "OpenEXR/IlmImf/ImfTiledOutputFile.h",
  "OpenEXR/IlmImf/ImfTiledOutputPart.h",
  "OpenEXR/IlmImf/ImfTiledRgbaFile.h",
  "OpenEXR/IlmImf/ImfTimeCode.h",
  "OpenEXR/IlmImf/ImfTimeCodeAttribute.h",
  "OpenEXR/IlmImf/ImfVecAttribute.h",
  "OpenEXR/IlmImf/ImfVersion.h",
  "OpenEXR/IlmImf/ImfWav.h",
  "OpenEXR/IlmImf/ImfXdr.h",
  "OpenEXR/IlmImf/ImfZip.h",
  "OpenEXR/IlmImf/ImfZipCompressor.h",
  "OpenEXR/IlmImf/b44ExpLogTable.h",
  "OpenEXR/IlmImf/dwaLookups.h",
  "OpenEXR/IlmImfUtil/ImfDeepImage.h",
  "OpenEXR/IlmImfUtil/ImfDeepImageChannel.h",
  "OpenEXR/IlmImfUtil/ImfDeepImageIO.h",
  "OpenEXR/IlmImfUtil/ImfDeepImageLevel.h",
  "OpenEXR/IlmImfUtil/ImfFlatImage.h",
  "OpenEXR/IlmImfUtil/ImfFlatImageChannel.h",
  "OpenEXR/IlmImfUtil/ImfFlatImageIO.h",
  "OpenEXR/IlmImfUtil/ImfFlatImageLevel.h",
  "OpenEXR/IlmImfUtil/ImfImage.h",
  "OpenEXR/IlmImfUtil/ImfImageChannel.h",
  "OpenEXR/IlmImfUtil/ImfImageChannelRenaming.h",
  "OpenEXR/IlmImfUtil/ImfImageDataWindow.h",
  "OpenEXR/IlmImfUtil/ImfImageIO.h",
  "OpenEXR/IlmImfUtil/ImfImageLevel.h",
  "OpenEXR/IlmImfUtil/ImfSampleCountChannel.h",
  "OpenEXR/config.windows/OpenEXRConfig.h"
 } )



configuration( {} )
vpaths { ["Source"] = { "**.c", "**.cpp", "**.cs", "**.asm" } }
files( {
  "IlmBase/Half/half.cpp",
  "IlmBase/Iex/IexBaseExc.cpp",
  "IlmBase/Iex/IexThrowErrnoExc.cpp",
  "IlmBase/IexMath/IexMathFloatExc.cpp",
  "IlmBase/IexMath/IexMathFpu.cpp",
  "IlmBase/IlmThread/IlmThread.cpp",
  "IlmBase/IlmThread/IlmThreadMutex.cpp",
  "IlmBase/IlmThread/IlmThreadMutexPosix.cpp",
  "IlmBase/IlmThread/IlmThreadMutexWin32.cpp",
  "IlmBase/IlmThread/IlmThreadPool.cpp",
  "IlmBase/IlmThread/IlmThreadPosix.cpp",
  "IlmBase/IlmThread/IlmThreadSemaphore.cpp",
  "IlmBase/IlmThread/IlmThreadSemaphorePosix.cpp",
  "IlmBase/IlmThread/IlmThreadSemaphorePosixCompat.cpp",
  "IlmBase/IlmThread/IlmThreadSemaphoreWin32.cpp",
  "IlmBase/IlmThread/IlmThreadWin32.cpp",
  "IlmBase/Imath/ImathBox.cpp",
  "IlmBase/Imath/ImathColorAlgo.cpp",
  "IlmBase/Imath/ImathFun.cpp",
  "IlmBase/Imath/ImathMatrixAlgo.cpp",
  "IlmBase/Imath/ImathRandom.cpp",
  "IlmBase/Imath/ImathShear.cpp",
  "IlmBase/Imath/ImathVec.cpp",
  "OpenEXR/IlmImf/ImfAcesFile.cpp",
  "OpenEXR/IlmImf/ImfAttribute.cpp",
  "OpenEXR/IlmImf/ImfB44Compressor.cpp",
  "OpenEXR/IlmImf/ImfBoxAttribute.cpp",
  "OpenEXR/IlmImf/ImfCRgbaFile.cpp",
  "OpenEXR/IlmImf/ImfChannelList.cpp",
  "OpenEXR/IlmImf/ImfChannelListAttribute.cpp",
  "OpenEXR/IlmImf/ImfChromaticities.cpp",
  "OpenEXR/IlmImf/ImfChromaticitiesAttribute.cpp",
  "OpenEXR/IlmImf/ImfCompositeDeepScanLine.cpp",
  "OpenEXR/IlmImf/ImfCompressionAttribute.cpp",
  "OpenEXR/IlmImf/ImfCompressor.cpp",
  "OpenEXR/IlmImf/ImfConvert.cpp",
  "OpenEXR/IlmImf/ImfDeepCompositing.cpp",
  "OpenEXR/IlmImf/ImfDeepFrameBuffer.cpp",
  "OpenEXR/IlmImf/ImfDeepImageStateAttribute.cpp",
  "OpenEXR/IlmImf/ImfDeepScanLineInputFile.cpp",
  "OpenEXR/IlmImf/ImfDeepScanLineInputPart.cpp",
  "OpenEXR/IlmImf/ImfDeepScanLineOutputFile.cpp",
  "OpenEXR/IlmImf/ImfDeepScanLineOutputPart.cpp",
  "OpenEXR/IlmImf/ImfDeepTiledInputFile.cpp",
  "OpenEXR/IlmImf/ImfDeepTiledInputPart.cpp",
  "OpenEXR/IlmImf/ImfDeepTiledOutputFile.cpp",
  "OpenEXR/IlmImf/ImfDeepTiledOutputPart.cpp",
  "OpenEXR/IlmImf/ImfDoubleAttribute.cpp",
  "OpenEXR/IlmImf/ImfDwaCompressor.cpp",
  "OpenEXR/IlmImf/ImfEnvmap.cpp",
  "OpenEXR/IlmImf/ImfEnvmapAttribute.cpp",
  "OpenEXR/IlmImf/ImfFastHuf.cpp",
  "OpenEXR/IlmImf/ImfFloatAttribute.cpp",
  "OpenEXR/IlmImf/ImfFloatVectorAttribute.cpp",
  "OpenEXR/IlmImf/ImfFrameBuffer.cpp",
  "OpenEXR/IlmImf/ImfFramesPerSecond.cpp",
  "OpenEXR/IlmImf/ImfGenericInputFile.cpp",
  "OpenEXR/IlmImf/ImfGenericOutputFile.cpp",
  "OpenEXR/IlmImf/ImfHeader.cpp",
  "OpenEXR/IlmImf/ImfHuf.cpp",
  "OpenEXR/IlmImf/ImfIO.cpp",
  "OpenEXR/IlmImf/ImfInputFile.cpp",
  "OpenEXR/IlmImf/ImfInputPart.cpp",
  "OpenEXR/IlmImf/ImfInputPartData.cpp",
  "OpenEXR/IlmImf/ImfIntAttribute.cpp",
  "OpenEXR/IlmImf/ImfKeyCode.cpp",
  "OpenEXR/IlmImf/ImfKeyCodeAttribute.cpp",
  "OpenEXR/IlmImf/ImfLineOrderAttribute.cpp",
  "OpenEXR/IlmImf/ImfLut.cpp",
  "OpenEXR/IlmImf/ImfMatrixAttribute.cpp",
  "OpenEXR/IlmImf/ImfMisc.cpp",
  "OpenEXR/IlmImf/ImfMultiPartInputFile.cpp",
  "OpenEXR/IlmImf/ImfMultiPartOutputFile.cpp",
  "OpenEXR/IlmImf/ImfMultiView.cpp",
  "OpenEXR/IlmImf/ImfOpaqueAttribute.cpp",
  "OpenEXR/IlmImf/ImfOutputFile.cpp",
  "OpenEXR/IlmImf/ImfOutputPart.cpp",
  "OpenEXR/IlmImf/ImfOutputPartData.cpp",
  "OpenEXR/IlmImf/ImfPartType.cpp",
  "OpenEXR/IlmImf/ImfPizCompressor.cpp",
  "OpenEXR/IlmImf/ImfPreviewImage.cpp",
  "OpenEXR/IlmImf/ImfPreviewImageAttribute.cpp",
  "OpenEXR/IlmImf/ImfPxr24Compressor.cpp",
  "OpenEXR/IlmImf/ImfRational.cpp",
  "OpenEXR/IlmImf/ImfRationalAttribute.cpp",
  "OpenEXR/IlmImf/ImfRgbaFile.cpp",
  "OpenEXR/IlmImf/ImfRgbaYca.cpp",
  "OpenEXR/IlmImf/ImfRle.cpp",
  "OpenEXR/IlmImf/ImfRleCompressor.cpp",
  "OpenEXR/IlmImf/ImfScanLineInputFile.cpp",
  "OpenEXR/IlmImf/ImfStandardAttributes.cpp",
  "OpenEXR/IlmImf/ImfStdIO.cpp",
  "OpenEXR/IlmImf/ImfStringAttribute.cpp",
  "OpenEXR/IlmImf/ImfStringVectorAttribute.cpp",
  "OpenEXR/IlmImf/ImfSystemSpecific.cpp",
  "OpenEXR/IlmImf/ImfTestFile.cpp",
  "OpenEXR/IlmImf/ImfThreading.cpp",
  "OpenEXR/IlmImf/ImfTileDescriptionAttribute.cpp",
  "OpenEXR/IlmImf/ImfTileOffsets.cpp",
  "OpenEXR/IlmImf/ImfTiledInputFile.cpp",
  "OpenEXR/IlmImf/ImfTiledInputPart.cpp",
  "OpenEXR/IlmImf/ImfTiledMisc.cpp",
  "OpenEXR/IlmImf/ImfTiledOutputFile.cpp",
  "OpenEXR/IlmImf/ImfTiledOutputPart.cpp",
  "OpenEXR/IlmImf/ImfTiledRgbaFile.cpp",
  "OpenEXR/IlmImf/ImfTimeCode.cpp",
  "OpenEXR/IlmImf/ImfTimeCodeAttribute.cpp",
  "OpenEXR/IlmImf/ImfVecAttribute.cpp",
  "OpenEXR/IlmImf/ImfVersion.cpp",
  "OpenEXR/IlmImf/ImfWav.cpp",
  "OpenEXR/IlmImf/ImfZip.cpp",
  "OpenEXR/IlmImf/ImfZipCompressor.cpp",
  "OpenEXR/IlmImfUtil/ImfDeepImage.cpp",
  "OpenEXR/IlmImfUtil/ImfDeepImageChannel.cpp",
  "OpenEXR/IlmImfUtil/ImfDeepImageIO.cpp",
  "OpenEXR/IlmImfUtil/ImfDeepImageLevel.cpp",
  "OpenEXR/IlmImfUtil/ImfFlatImage.cpp",
  "OpenEXR/IlmImfUtil/ImfFlatImageChannel.cpp",
  "OpenEXR/IlmImfUtil/ImfFlatImageIO.cpp",
  "OpenEXR/IlmImfUtil/ImfFlatImageLevel.cpp",
  "OpenEXR/IlmImfUtil/ImfImage.cpp",
  "OpenEXR/IlmImfUtil/ImfImageChannel.cpp",
  "OpenEXR/IlmImfUtil/ImfImageDataWindow.cpp",
  "OpenEXR/IlmImfUtil/ImfImageIO.cpp",
  "OpenEXR/IlmImfUtil/ImfImageLevel.cpp",
  "OpenEXR/IlmImfUtil/ImfSampleCountChannel.cpp"
 } )


configuration( {} )
includedirs( { "IlmBase", "OpenEXR", "IlmBase/Half", "IlmBase/Iex", "IlmBase/IexMath", "IlmBase/IlmThread", "IlmBase/Imath", "IlmBase/config.windows", "OpenEXR/IlmImf", "OpenEXR/IlmImfUtil", "OpenEXR/config.windows" } )

configuration( { "LINUX32" } )
includedirs( { "../zlib" } )

configuration( { "LINUX64" } )
includedirs( { "../zlib" } )

configuration( { "WIN32" } )
includedirs( { "../zlib" } )

configuration( { "WIN64" } )
includedirs( { "../zlib" } )
