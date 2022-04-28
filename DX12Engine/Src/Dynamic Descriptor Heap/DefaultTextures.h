//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author(s):  James Stanard
//

#pragma once

#include "GpuResource.h"
#include "Utility.h"
#include "Texture.h"
#include "DDSTextureLoader.h"
#include "../Singleton.h"
// A referenced-counted pointer to a Texture.  See methods below.
class TextureRef;
class Direct3D;
//
// Texture file loading system.
//
// References to textures are passed around so that a texture may be shared.  When
// all references to a texture expire, the texture memory is reclaimed.
//
namespace TextureManager
{
    enum eDefaultTexture
    {
        kMagenta2D,  // Useful for indicating missing textures
        kBlackOpaque2D,
        kBlackTransparent2D,
        kWhiteOpaque2D,
        kWhiteTransparent2D,
        kDefaultNormalMap,
        kBlackCubeMap,

        kNumDefaultTextures
    };



    void Initialize( const std::wstring& RootPath );
    void Shutdown(void);

    // Load a texture from a DDS file.  Never returns null references, but if a 
    // texture cannot be found, ref->IsValid() will return false.
    TextureRef LoadDDSFromFile(Direct3D* direct_3d, const std::wstring& filePath, eDefaultTexture fallback = kMagenta2D, bool sRGB = false );
    TextureRef LoadDDSFromFile(Direct3D* direct_3d, const std::string& filePath, eDefaultTexture fallback = kMagenta2D, bool sRGB = false );

    class DefaultTextures : public Singleton<DefaultTextures>
	{
        friend class Singleton<DefaultTextures>;
    public:
        void Init(Direct3D* direct_3d)
        {
            uint32_t MagentaPixel = 0xFFFF00FF;
            DefaultTexturesArray[kMagenta2D].Create2D(direct_3d, 4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &MagentaPixel);
            uint32_t BlackOpaqueTexel = 0xFF000000;
            DefaultTexturesArray[kBlackOpaque2D].Create2D(direct_3d, 4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &BlackOpaqueTexel);
            uint32_t BlackTransparentTexel = 0x00000000;
            DefaultTexturesArray[kBlackTransparent2D].Create2D(direct_3d, 4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &BlackTransparentTexel);
            uint32_t WhiteOpaqueTexel = 0xFFFFFFFF;
            DefaultTexturesArray[kWhiteOpaque2D].Create2D(direct_3d, 4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &WhiteOpaqueTexel);
            uint32_t WhiteTransparentTexel = 0x00FFFFFF;
            DefaultTexturesArray[kWhiteTransparent2D].Create2D(direct_3d, 4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &WhiteTransparentTexel);
            uint32_t FlatNormalTexel = 0x00FF8080;
            DefaultTexturesArray[kDefaultNormalMap].Create2D(direct_3d, 4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &FlatNormalTexel);
            uint32_t BlackCubeTexels[6] = {};
            DefaultTexturesArray[kBlackCubeMap].CreateCube(direct_3d, 4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, BlackCubeTexels);
        }
       
        D3D12_CPU_DESCRIPTOR_HANDLE GetDefaultTexture(eDefaultTexture texID)
        {
            ASSERT(texID < kNumDefaultTextures);
            return DefaultTexturesArray[texID].GetSRV();
        }

    private:
        Texture DefaultTexturesArray[kNumDefaultTextures];
    };
}

// Forward declaration; private implementation
class ManagedTexture;

//
// A handle to a ManagedTexture.  Constructors and destructors modify the reference
// count.  When the last reference is destroyed, the DefaultTextures is informed that
// the texture should be deleted.
//
class TextureRef
{
public:

    TextureRef( const TextureRef& ref );
    TextureRef( ManagedTexture* tex = nullptr );
    ~TextureRef();

    void operator= (std::nullptr_t);
    void operator= (TextureRef& rhs);

    // Check that this points to a valid texture (which loaded successfully)
    bool IsValid() const;

    // Gets the SRV descriptor handle.  If the reference is invalid,
    // returns a valid descriptor handle (specified by the fallback)
    D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const;

    // Get the texture pointer.  Client is responsible to not dereference
    // null pointers.
    const Texture* Get( void ) const;

    const Texture* operator->( void ) const;

private:
    ManagedTexture* m_ref;
};
