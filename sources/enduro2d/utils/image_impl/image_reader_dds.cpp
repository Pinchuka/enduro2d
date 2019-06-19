/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "image_impl.hpp"

namespace
{
    using namespace e2d;
    // from https://github.com/tlorach/nvFX/blob/master/samples/shared/nv_dds/nv_dds.h

    // pixel format flags
    const u32 ddsf_alphapixels = 0x00000001;
    const u32 ddsf_fourcc = 0x00000004;
    const u32 ddsf_rgb = 0x00000040;
    const u32 ddsf_rgba = 0x00000041;

    // dwCaps2 flags
    const u32 ddsf_cubemap = 0x00000200;
    const u32 ddsf_volume = 0x00200000;

    // compressed texture types
    const u32 fourcc_dxt1 = 0x31545844; //(MAKEFOURCC('D','X','T','1'))
    const u32 fourcc_dxt3 = 0x33545844; //(MAKEFOURCC('D','X','T','3'))
    const u32 fourcc_dxt5 = 0x35545844; //(MAKEFOURCC('D','X','T','5'))

    struct DXTColBlock
    {
        u16 col0;
        u16 col1;

        u8 row[4];
    };

    struct DXT3AlphaBlock
    {
        u16 row[4];
    };

    struct DXT5AlphaBlock
    {
        u8 alpha0;
        u8 alpha1;

        u8 row[6];
    };

    struct dds_pixelformat
    {
        u32 dwSize;
        u32 dwFlags;
        u32 dwFourCC;
        u32 dwRGBBitCount;
        u32 dwRBitMask;
        u32 dwGBitMask;
        u32 dwBBitMask;
        u32 dwABitMask;
    };

    struct dds_header
    {
        u32 dwSize;
        u32 dwFlags;
        u32 dwHeight;
        u32 dwWidth;
        u32 dwPitchOrLinearSize;
        u32 dwDepth;
        u32 dwMipMapCount;
        u32 dwReserved1[11];
        dds_pixelformat ddspf;
        u32 dwCaps1;
        u32 dwCaps2;
        u32 dwReserved2[3];
    };

    struct dds_header_with_magic
    {
        u32         dwMagicFourCC;
        dds_header  header;
    };

    static_assert(sizeof(dds_pixelformat) == 32, "invalid DDSS pixelformat structure size");
    static_assert(sizeof(dds_header) == 124, "invalid DDS header size");

   bool is_dds(const void* data, std::size_t byte_size) {
        if ( byte_size > sizeof(dds_header_with_magic) ) {
            const auto* hdr = (const dds_header_with_magic*) data;
            return hdr->dwMagicFourCC == 0x20534444; // DDS
        }
        return false;
    }

    bool get_dds_format(const dds_header& hdr, image_data_format& out_format, u32& out_bytes_per_block) {
        out_format = image_data_format(-1);
        out_bytes_per_block = 0;
        if ( math::check_all_flags(hdr.ddspf.dwFlags, ddsf_fourcc) ) {
            switch (hdr.ddspf.dwFourCC)
            {
            case fourcc_dxt1:
                out_format = math::check_all_flags(hdr.ddspf.dwFlags, ddsf_alphapixels)
                                ? image_data_format::rgba_dxt1
                                : image_data_format::rgb_dxt1;
                out_bytes_per_block = 8;
                break;
            case fourcc_dxt3:
                out_format = image_data_format::rgba_dxt3;
                out_bytes_per_block = 16;
                break;
            case fourcc_dxt5:
                out_format = image_data_format::rgba_dxt5;
                out_bytes_per_block = 16;
                break;
            default:
                return false; // unsupported compressed format
            }
        } else if ( math::check_all_flags(hdr.ddspf.dwFlags, ddsf_rgba|ddsf_rgb) && (hdr.ddspf.dwRGBBitCount == 32) ) {
            out_bytes_per_block = 4;
            out_format = image_data_format::rgba8;
            if ( hdr.ddspf.dwRBitMask == 0x00FF0000 ) {
                return false; // BGRA format is not supported
            }
        } else if ( math::check_all_flags(hdr.ddspf.dwFlags, ddsf_rgb) && (hdr.ddspf.dwRGBBitCount == 24) ) {
            out_bytes_per_block = 3;
            out_format = image_data_format::rgb8;
            if (hdr.ddspf.dwRBitMask == 0x00FF0000) {
                return false; // BGRA format is not supported
            }
        } else if ( hdr.ddspf.dwRGBBitCount == 16 ) {
            return false; // 16-bit format is not supported by engine
        } else if ( hdr.ddspf.dwRGBBitCount == 8 ) {
            out_format = image_data_format::g8;
            out_bytes_per_block = 1;
        }
        return true;
    }
}

namespace e2d::images::impl
{
    bool try_load_image_dds(image& dst, const buffer& src) noexcept {
        if ( !is_dds(src.data(), src.size()) ) {
            return false;
        }
        const dds_header& hdr = ((const dds_header_with_magic*)src.data())->header;
        const u8* content = src.data() + sizeof(dds_header_with_magic);
        if ( math::check_all_flags(ddsf_cubemap, hdr.dwCaps2) ||
             math::check_all_flags(ddsf_volume, hdr.dwCaps2) ||
             (hdr.dwDepth > 0) ) {
            return false; // cubemap and volume textures are not supported
        }
        image_data_format format = image_data_format(-1);
        u32 bytes_per_block = 0;
        if ( !get_dds_format(hdr, format, bytes_per_block) ) {
            return false; // unsupported format
        }
        //u32 num_mipmaps = (hdr.dwMipMapCount == 0) ? 1 : hdr.dwMipMapCount;
        v2u dimension = v2u(hdr.dwWidth, hdr.dwHeight);
        std::size_t size;
        switch (format)
        {
        case image_data_format::rgb_dxt1:
        case image_data_format::rgba_dxt3:
        case image_data_format::rgba_dxt5:
            size = ((dimension.x+3)/4) * ((dimension.y+3)/4) * bytes_per_block;
            break;
        default:
            size = dimension.x * dimension.y * bytes_per_block;
            break;
        }
        dst = image(dimension, format, buffer(content, size));
        return true;
    }
}
