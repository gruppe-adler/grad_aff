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
#include <cstdint>

//#define GRAD_AFF_USE_CPP11_THREADS

#ifdef GRAD_AFF_USE_CPP11_THREADS
    #include <thread>
#elif defined GRAD_AFF_USE_CPP17_PARALLELISM
    #include <execution>
#endif


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
#ifdef GRAD_AFF_USE_CPP11_THREADS
template<typename Iterator, class Function>
void parallel_for(const Iterator& first, const Iterator& last, Function&& f, const int nthreads = 1, const int threshold = 1000)
{
    const unsigned int group = std::max(std::max(ptrdiff_t(1), ptrdiff_t(std::abs(threshold))), ((last - first)) / std::abs(nthreads));
    std::vector<std::thread> threads;
    threads.reserve(nthreads);
    Iterator it = first;
    for (; it < last - group; it += group) {
        threads.push_back(std::thread([=, &f]() {std::for_each(it, std::min(it + group, last), f); }));
    }
    std::for_each(it, last, f);

    std::for_each(threads.begin(), threads.end(), [](std::thread& x) {x.join(); });
}

static void compressImage(uint8_t const* rgba, int width, int height, int pitch, void* blocks, int flags)
{
    flags = fixFlags(flags);

    auto nThreads = std::thread::hardware_concurrency();

    std::vector<size_t> indexIterator;
    indexIterator.reserve(height / 4);
    for (size_t i = 0; i < height; i += 4) {
        indexIterator.push_back(i);
    }

    auto worker = [blocks, flags, width, height, rgba, pitch](std::vector<size_t>::iterator begin, std::vector<size_t>::iterator end) {
        for (auto it = begin; it != end; ++it) {
            auto y = *it;
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
    };


    std::vector<std::thread> threads(nThreads);
    const int grainsize = indexIterator.size() / nThreads;
    auto work_iter = std::begin(indexIterator);
    for (auto it = std::begin(threads); it != std::end(threads) - 1; ++it) {
        *it = std::thread(worker, work_iter, work_iter + grainsize);
        work_iter += grainsize;
    }
    threads.back() = std::thread(worker, work_iter, std::end(indexIterator));

    for (auto&& i : threads) {
        i.join();
    }
}
#elif defined GRAD_AFF_USE_CPP17_PARALLELISM
// orignal method with some optimizations
static void compressImage(uint8_t const* rgba, int width, int height, int pitch, void* blocks, int flags)
{
    flags = fixFlags(flags);

    std::vector<size_t> indexIterator;
    indexIterator.reserve(height / 4);
    for (size_t i = 0; i < height; i += 4) {
        indexIterator.push_back(i);
    }

    std::for_each(std::execution::par, indexIterator.begin(), indexIterator.end(), [blocks, flags, width, height, rgba, pitch](size_t y)
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
    });
}
#else
static void compressImage(uint8_t const* rgba, int width, int height, int pitch, void* blocks, int flags)
{
    flags = fixFlags(flags);

    for (size_t y = 0; y < height; y += 4)
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
#endif