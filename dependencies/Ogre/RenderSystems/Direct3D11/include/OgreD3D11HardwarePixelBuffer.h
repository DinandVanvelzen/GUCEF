/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2016 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef __D3D11PIXELBUFFER_H__
#define __D3D11PIXELBUFFER_H__

#include "OgreD3D11Prerequisites.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreD3D11Driver.h"

struct ID3D11Resource;

namespace Ogre {

    class D3D11HardwarePixelBuffer: public HardwarePixelBuffer
    {
    protected:
        /// Lock a box
        PixelBox lockImpl(const Image::Box &lockBox, LockOptions options);

        /// Unlock a box
        void unlockImpl(void);

        /// D3DDevice pointer
        D3D11Device & mDevice;

        D3D11Texture * mParentTexture;
        const UINT mFace;
        const UINT mMipLevel;

        // if the usage is static - alloc at lock then use device UpdateSubresource when unlock and free memory
        vector<int8>::type mDataForStaticUsageLock; 

        Image::Box mLockBox;
        LockOptions mCurrentLockOptions;

        /// Render targets
        typedef vector<RenderTexture*>::type SliceTRT;
        SliceTRT mSliceTRT;

        void createStagingBuffer();
        bool mUsingStagingBuffer;
        ComPtr<ID3D11Resource> mStagingBuffer;
        
        void _map(ID3D11Resource *res, D3D11_MAP flags, PixelBox & box);
        void _mapstagingbuffer(D3D11_MAP flags, PixelBox &box);

        void _unmap(ID3D11Resource *res);
        void _unmapstagingbuffer(bool copyback = true);
        void _unmapstaticbuffer();
    public:
        D3D11HardwarePixelBuffer(D3D11Texture * parentTexture, D3D11Device & device, UINT mipLevel,
            size_t width, size_t height, size_t depth, UINT face, PixelFormat format, HardwareBuffer::Usage usage);

        /// @copydoc HardwarePixelBuffer::blit
        void blit(const HardwarePixelBufferSharedPtr &src, const Image::Box &srcBox, const Image::Box &dstBox);

        /// @copydoc HardwarePixelBuffer::blitFromMemory
        void blitFromMemory(const PixelBox &src, const Image::Box &dstBox);

        /// @copydoc HardwarePixelBuffer::blitToMemory
        void blitToMemory(const Image::Box &srcBox, const PixelBox &dst);

        /// Internal function to update mipmaps on update of level 0
        void _genMipmaps();


        ~D3D11HardwarePixelBuffer();

        /// Get rendertarget for z slice
        RenderTexture *getRenderTarget(size_t zoffset);

        /// Notify TextureBuffer of destruction of render target
        virtual void _clearSliceRTT(size_t zoffset)
        {
            if (mSliceTRT.size() > zoffset)
            {
                mSliceTRT[zoffset] = 0;
            }
        }

        D3D11Texture * getParentTexture() const;

        UINT getFace() const;
        UINT getSubresourceIndex(size_t box_front) const;
        D3D11_BOX getSubresourceBox(const Image::Box &box) const;
    };
};
#endif
