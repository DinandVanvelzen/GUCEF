#-------------------------------------------------------------------
# This file has been automatically generated by ProjectGenerator    
# which is part of a build system designed for GUCEF                
#     (Galaxy Unlimited Framework)                                  
# For the latest info, see http://www.VanvelzenSoftware.com/        
#                                                                   
# The contents of this file are placed in the public domain. Feel   
# free to make use of it in any way you like.                       
#-------------------------------------------------------------------


ifndef MY_MODULE_PATH
  MY_MODULE_PATH := $(call my-dir)
endif
LOCAL_PATH := $(MY_MODULE_PATH)

include $(CLEAR_VARS)

@echo Module path: $(MY_MODULE_PATH)
LOCAL_MODULE := Ogre
LOCAL_MODULE_FILENAME := libOgre
@echo Module name: $(LOCAL_MODULE)

LOCAL_SRC_FILES := \
  src/OgreAlignedAllocator.cpp \
  src/OgreAnimable.cpp \
  src/OgreAnimation.cpp \
  src/OgreAnimationState.cpp \
  src/OgreAnimationTrack.cpp \
  src/OgreArchive.cpp \
  src/OgreArchiveManager.cpp \
  src/OgreAtomicScalar.cpp \
  src/OgreAutoParamDataSource.cpp \
  src/OgreAxisAlignedBox.cpp \
  src/OgreBillboard.cpp \
  src/OgreBillboardChain.cpp \
  src/OgreBillboardParticleRenderer.cpp \
  src/OgreBillboardSet.cpp \
  src/OgreBone.cpp \
  src/OgreCamera.cpp \
  src/OgreCodec.cpp \
  src/OgreColourValue.cpp \
  src/OgreCommon.cpp \
  src/OgreCompositionPass.cpp \
  src/OgreCompositionTargetPass.cpp \
  src/OgreCompositionTechnique.cpp \
  src/OgreCompositor.cpp \
  src/OgreCompositorChain.cpp \
  src/OgreCompositorInstance.cpp \
  src/OgreCompositorManager.cpp \
  src/OgreConfigDialogNoOp.cpp \
  src/OgreConfigFile.cpp \
  src/OgreControllerManager.cpp \
  src/OgreConvexBody.cpp \
  src/OgreDDSCodec.cpp \
  src/OgreDataStream.cpp \
  src/OgreDefaultHardwareBufferManager.cpp \
  src/OgreDefaultSceneQueries.cpp \
  src/OgreDeflate.cpp \
  src/OgreDepthBuffer.cpp \
  src/OgreDistanceLodStrategy.cpp \
  src/OgreDualQuaternion.cpp \
  src/OgreDynLib.cpp \
  src/OgreDynLibManager.cpp \
  src/OgreETCCodec.cpp \
  src/OgreEdgeListBuilder.cpp \
  src/OgreEntity.cpp \
  src/OgreErrorDialogNoOp.cpp \
  src/OgreException.cpp \
  src/OgreExternalTextureSource.cpp \
  src/OgreExternalTextureSourceManager.cpp \
  src/OgreFileSystem.cpp \
  src/OgreFrustum.cpp \
  src/OgreGpuProgram.cpp \
  src/OgreGpuProgramManager.cpp \
  src/OgreGpuProgramParams.cpp \
  src/OgreGpuProgramUsage.cpp \
  src/OgreHardwareBufferManager.cpp \
  src/OgreHardwareCounterBuffer.cpp \
  src/OgreHardwareIndexBuffer.cpp \
  src/OgreHardwareOcclusionQuery.cpp \
  src/OgreHardwarePixelBuffer.cpp \
  src/OgreHardwareUniformBuffer.cpp \
  src/OgreHardwareVertexBuffer.cpp \
  src/OgreHighLevelGpuProgram.cpp \
  src/OgreHighLevelGpuProgramManager.cpp \
  src/OgreImage.cpp \
  src/OgreInstanceBatch.cpp \
  src/OgreInstanceBatchHW.cpp \
  src/OgreInstanceBatchHW_VTF.cpp \
  src/OgreInstanceBatchShader.cpp \
  src/OgreInstanceBatchVTF.cpp \
  src/OgreInstanceManager.cpp \
  src/OgreInstancedEntity.cpp \
  src/OgreInstancedGeometry.cpp \
  src/OgreKeyFrame.cpp \
  src/OgreLight.cpp \
  src/OgreLodStrategy.cpp \
  src/OgreLodStrategyManager.cpp \
  src/OgreLog.cpp \
  src/OgreLogManager.cpp \
  src/OgreManualObject.cpp \
  src/OgreMaterial.cpp \
  src/OgreMaterialManager.cpp \
  src/OgreMaterialSerializer.cpp \
  src/OgreMath.cpp \
  src/OgreMatrix3.cpp \
  src/OgreMatrix4.cpp \
  src/OgreMemoryAllocatedObject.cpp \
  src/OgreMemoryNedAlloc.cpp \
  src/OgreMemoryNedPooling.cpp \
  src/OgreMemoryTracker.cpp \
  src/OgreMesh.cpp \
  src/OgreMeshManager.cpp \
  src/OgreMeshSerializer.cpp \
  src/OgreMeshSerializerImpl.cpp \
  src/OgreMovableObject.cpp \
  src/OgreMovablePlane.cpp \
  src/OgreNode.cpp \
  src/OgreNumerics.cpp \
  src/OgreOptimisedUtil.cpp \
  src/OgreOptimisedUtilGeneral.cpp \
  src/OgreOptimisedUtilSSE.cpp \
  src/OgrePVRTCCodec.cpp \
  src/OgreParticle.cpp \
  src/OgreParticleEmitter.cpp \
  src/OgreParticleEmitterCommands.cpp \
  src/OgreParticleIterator.cpp \
  src/OgreParticleSystem.cpp \
  src/OgreParticleSystemManager.cpp \
  src/OgrePass.cpp \
  src/OgrePatchMesh.cpp \
  src/OgrePatchSurface.cpp \
  src/OgrePixelCountLodStrategy.cpp \
  src/OgrePixelFormat.cpp \
  src/OgrePlane.cpp \
  src/OgrePlatformInformation.cpp \
  src/OgrePolygon.cpp \
  src/OgrePose.cpp \
  src/OgrePrecompiledHeaders.cpp \
  src/OgrePredefinedControllers.cpp \
  src/OgrePrefabFactory.cpp \
  src/OgreProfiler.cpp \
  src/OgreQuaternion.cpp \
  src/OgreRectangle2D.cpp \
  src/OgreRenderQueue.cpp \
  src/OgreRenderQueueInvocation.cpp \
  src/OgreRenderQueueSortingGrouping.cpp \
  src/OgreRenderSystem.cpp \
  src/OgreRenderSystemCapabilities.cpp \
  src/OgreRenderSystemCapabilitiesManager.cpp \
  src/OgreRenderSystemCapabilitiesSerializer.cpp \
  src/OgreRenderTarget.cpp \
  src/OgreRenderTexture.cpp \
  src/OgreRenderToVertexBuffer.cpp \
  src/OgreRenderWindow.cpp \
  src/OgreResource.cpp \
  src/OgreResourceBackgroundQueue.cpp \
  src/OgreResourceGroupManager.cpp \
  src/OgreResourceManager.cpp \
  src/OgreRibbonTrail.cpp \
  src/OgreRoot.cpp \
  src/OgreRotationSpline.cpp \
  src/OgreSTBICodec.cpp \
  src/OgreSceneManager.cpp \
  src/OgreSceneManagerEnumerator.cpp \
  src/OgreSceneNode.cpp \
  src/OgreSceneQuery.cpp \
  src/OgreScriptCompiler.cpp \
  src/OgreScriptLexer.cpp \
  src/OgreScriptParser.cpp \
  src/OgreScriptTranslator.cpp \
  src/OgreSerializer.cpp \
  src/OgreShadowCameraSetup.cpp \
  src/OgreShadowCameraSetupFocused.cpp \
  src/OgreShadowCameraSetupLiSPSM.cpp \
  src/OgreShadowCameraSetupPSSM.cpp \
  src/OgreShadowCameraSetupPlaneOptimal.cpp \
  src/OgreShadowCaster.cpp \
  src/OgreShadowTextureManager.cpp \
  src/OgreShadowVolumeExtrudeProgram.cpp \
  src/OgreSimpleRenderable.cpp \
  src/OgreSimpleSpline.cpp \
  src/OgreSkeleton.cpp \
  src/OgreSkeletonInstance.cpp \
  src/OgreSkeletonManager.cpp \
  src/OgreSkeletonSerializer.cpp \
  src/OgreStaticGeometry.cpp \
  src/OgreStreamSerialiser.cpp \
  src/OgreString.cpp \
  src/OgreStringConverter.cpp \
  src/OgreStringInterface.cpp \
  src/OgreSubEntity.cpp \
  src/OgreSubMesh.cpp \
  src/OgreTagPoint.cpp \
  src/OgreTangentSpaceCalc.cpp \
  src/OgreTechnique.cpp \
  src/OgreTexture.cpp \
  src/OgreTextureManager.cpp \
  src/OgreTextureUnitState.cpp \
  src/OgreUTFString.cpp \
  src/OgreUnifiedHighLevelGpuProgram.cpp \
  src/OgreUserObjectBindings.cpp \
  src/OgreVector2.cpp \
  src/OgreVector3.cpp \
  src/OgreVector4.cpp \
  src/OgreVertexIndexData.cpp \
  src/OgreViewport.cpp \
  src/OgreWindowEventUtilities.cpp \
  src/OgreWireBoundingBox.cpp \
  src/OgreWorkQueue.cpp \
  src/OgreZip.cpp \
  src/Android/OgreAPKFileSystemArchive.cpp \
  src/Android/OgreAPKZipArchive.cpp \
  src/Android/OgreAndroidLogListener.cpp \
  src/Android/OgreFileSystemLayer.cpp \
  src/Android/JNI/OgreActivityJNI.cpp \
  src/Hash/MurmurHash3.cpp \
  src/Threading/OgreBarrierPThreads.cpp \
  src/Threading/OgreDefaultWorkQueueStandard.cpp \
  src/Threading/OgreLightweightMutexPThreads.cpp \
  src/Threading/OgreThreadsPThreads.cpp \
  src/nedmalloc/nedmalloc.c

LOCAL_C_INCLUDES := \
  $(MY_MODULE_PATH)/include \
  $(MY_MODULE_PATH)/include/Android \
  $(MY_MODULE_PATH)/include/Hash \
  $(MY_MODULE_PATH)/include/Threading \
  $(MY_MODULE_PATH)/src \
  $(MY_MODULE_PATH)/src/nedmalloc \
  $(MY_MODULE_PATH)/src/stbi \
  $(MY_MODULE_PATH)/../../freetype/include \
  $(MY_MODULE_PATH)/../../freetype/include/freetype \
  $(MY_MODULE_PATH)/../../freetype/include/freetype/config \
  $(MY_MODULE_PATH)/../../freetype/include/freetype/internal \
  $(MY_MODULE_PATH)/../../freetype/include/freetype/internal/services \
  $(MY_MODULE_PATH)/../../freetype/src \
  $(MY_MODULE_PATH)/../../freetype/src/winfonts \
  $(MY_MODULE_PATH)/../include

LOCAL_CFLAGS := -DOGRE_NONCLIENT_BUILD -DOGRE_NO_CONFIG_DIALOG


LOCAL_SHARED_LIBRARIES := \
  freetype


LOCAL_LDLIBS := \
  -latomic \
  -ldl

include $(BUILD_SHARED_LIBRARY)

