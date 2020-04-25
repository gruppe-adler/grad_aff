/**

Copyright(c) 2006 Simon Brown                          si@sjbrown.co.uk

Permission is hereby granted, free of charge, to any person obtaining
a copy of this softwareand associated documentation files(the
    "Software"), to	deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and /or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to
    the following conditions :

The above copyright noticeand this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**/

#pragma once 

#include <squish.h>
#include <omp.h>

#include <cstdint>

static int fixFlags(int flags)
{
    // grab the flag bits
    int method = flags & (squish::kDxt1 | squish::kDxt3 | squish::kDxt5 | squish::kBc4 | squish::kBc5);
    int fit = flags & (squish::kColourIterativeClusterFit | squish::kColourClusterFit | squish::kColourRangeFit);
    int extra = flags & squish::kWeightColourByAlpha;

    // set defaults
    if (method != squish::kDxt3
        && method != squish::kDxt5
        && method != squish::kBc4
        && method != squish::kBc5)
    {
        method = squish::kDxt1;
    }
    if (fit != squish::kColourRangeFit && fit != squish::kColourIterativeClusterFit)
        fit = squish::kColourClusterFit;

    // done
    return method | fit | extra;
}

static void copyRGBA(uint8_t const* source, uint8_t* dest)
{
    for (int i = 0; i < 4; ++i)
        *dest++ = *source++;
}

// orignal method with some optimizations
static void compressImage(uint8_t const* rgba, int width, int height, int pitch, void* blocks, int flags)
{
    flags = fixFlags(flags);

    #pragma omp parallel for
    for (int y = 0; y < height; y += 4)
    {
        squish::u8* targetBlock = reinterpret_cast<squish::u8*>(blocks);
        int bytesPerBlock = ((flags & (squish::kDxt1 | squish::kBc4)) != 0) ? 8 : 16;
        targetBlock += ((y / 4) * ((width + 3) / 4)) * bytesPerBlock;

        for (int x = 0; x < width; x += 4)
        {
            squish::u8 sourceRgba[64];
            squish::u8* targetPixel = sourceRgba;
            int mask = 0;
            for (int py = 0; py < 4; ++py)
            {
                for (int px = 0; px < 4; ++px)
                {
                    int sx = x + px;
                    int sy = y + py;

                    if (sx < width && sy < height)
                    {
                        squish::u8 const* sourcePixel = rgba + pitch * sy + 4 * sx;
                        copyRGBA(sourcePixel, targetPixel);
                        mask |= (1 << (4 * py + px));
                    }

                    targetPixel += 4;
                }
            }
            squish::CompressMasked(sourceRgba, mask, targetBlock, flags, 0);
            targetBlock += bytesPerBlock;
        }
    }
}
