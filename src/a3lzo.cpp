#include "grad_aff/a3lzo.h"

// Based on https://community.bistudio.com/wiki/Compressed_LZO_File_Format
size_t grad_aff::Decompress(std::istream& i, std::vector<uint8_t>& output, size_t expectedSize)
{
    auto startPos = i.tellg();
    std::vector<uint8_t>::iterator op;
    unsigned int t = 0;
    std::vector<uint8_t>::iterator m_pos;

    auto op_end = output.end();
    op = output.begin();

    if (peekBytes<uint8_t>(i) > 17)
    {
        t = readBytes<uint8_t>(i) - 17U;
        if (t < 4) goto match_next;

        assert(t > 0);
        if ((op_end - op) < (t)) {
            throw std::overflow_error("Output Overun");
        }
        do *op++ = readBytes<uint8_t>(i); while (--t > 0);
        goto first_literal_run;
    }

B_3:
    t = (size_t)readBytes<uint8_t>(i);
    if (t >= 16) goto match;

    if (t == 0)
    {
        while (peekBytes<uint8_t>(i) == 0)
        {
            t += 255;
            i.seekg(1, std::ios_base::cur);
        }
        t += 15U + readBytes<uint8_t>(i);
    }
    assert(t > 0);
    if ((op_end - op) < (t + 3)) {
        throw std::overflow_error("Output Overrun");
    }

    *op++ = readBytes<uint8_t>(i);
    *op++ = readBytes<uint8_t>(i);
    *op++ = readBytes<uint8_t>(i);
    *op++ = readBytes<uint8_t>(i);
    if (--t > 0)
    {
        if (t >= 4)
        {
            do
            {
                *op++ = readBytes<uint8_t>(i);
                *op++ = readBytes<uint8_t>(i);
                *op++ = readBytes<uint8_t>(i);
                *op++ = readBytes<uint8_t>(i);
                t -= 4;
            } while (t >= 4);
            if (t > 0) do *op++ = readBytes<uint8_t>(i); while (--t > 0);
        }
        else
            do *op++ = readBytes<uint8_t>(i); while (--t > 0);
    }

first_literal_run:
    t = readBytes<uint8_t>(i);
    if (t >= 16) goto match;

    m_pos = op - (1 + M2_MAX_OFFSET);
    m_pos -= t >> 2;
    m_pos -= readBytes<uint8_t>(i) << 2;

    if (m_pos < output.begin() || m_pos >= op) {
        throw std::underflow_error("Lookbeding Overrun");
    }
    if ((op_end - op) < (3)) {
        throw std::overflow_error("Output Overrun");
    }
    *op++ = *m_pos++; *op++ = *m_pos++; *op++ = *m_pos;

    goto match_done;

match:
    if (t >= 64)
    {
        m_pos = op - 1;
        m_pos -= (t >> 2) & 7;
        m_pos -= readBytes<uint8_t>(i) << 3;
        t = (t >> 5) - 1;
        if (m_pos < output.begin() || m_pos >= op) {
            throw std::underflow_error("Lookbeding Overrun");
        }
        assert(t > 0);
        if ((op_end - op) < (t + 2)) {
            throw std::overflow_error("Output Overrun");
        }
        goto copy_match;
    }
    else if (t >= 32)
    {
        t &= 31;
        if (t == 0)
        {
            while (peekBytes<uint8_t>(i) == 0)
            {
                t += 255;
                i.seekg(1, std::ios_base::cur);
            }
            t += 31U + readBytes<uint8_t>(i);
        }

        m_pos = op - 1;
        m_pos -= (readBytes<uint8_t>(i, 0) >> 2) + (readBytes<uint8_t>(i, 1) << 6);

        i.seekg(2, std::ios_base::cur);
    }
    else if (t >= 16)
    {
        m_pos = op;
        m_pos -= (t & 8) << 11;

        t &= 7;
        if (t == 0)
        {
            while (peekBytes<uint8_t>(i) == 0)
            {
                t += 255;
                i.seekg(1, std::ios_base::cur);
            }
            t += 7U + readBytes<uint8_t>(i);
        }

        m_pos -= (readBytes<uint8_t>(i, 0) >> 2) + (readBytes<uint8_t>(i, 1) << 6);

        i.seekg(2, std::ios_base::cur);

        if (m_pos == op)
        {
            assert(t == 1);
            if (m_pos != op_end) {
                throw std::overflow_error("Output Overrun");
            }
            return i.tellg() - startPos;
        }
        m_pos -= 0x4000;
    }
    else
    {
        m_pos = op - 1;
        m_pos -= t >> 2;
        m_pos -= readBytes<uint8_t>(i) << 2;

        if (m_pos < output.begin() || m_pos >= op) {
            throw std::underflow_error("Lookbehind Overrun");
        }
        if ((op_end - op) < (2)) {
            throw std::overflow_error("Output Overrun");
        }
        *op++ = *m_pos++; *op++ = *m_pos;
        goto match_done;
    }

    if (m_pos < output.begin() || m_pos >= op) {
        throw std::underflow_error("Lookbehind Overrun");
    }
    assert(t > 0);
    if ((op_end - op) < (t + 2)) {
        throw std::overflow_error("Output Overrun");
    }

    if (t >= 2 * 4 - (3 - 1) && (op - m_pos) >= 4)
    {
        *op++ = *m_pos++;
        *op++ = *m_pos++;
        *op++ = *m_pos++;
        *op++ = *m_pos++;
        t -= 4 - (3 - 1);
        do
        {
            *op++ = *m_pos++;
            *op++ = *m_pos++;
            *op++ = *m_pos++;
            *op++ = *m_pos++;
            t -= 4;
        } while (t >= 4);
        if (t > 0) do *op++ = *m_pos++; while (--t > 0);
        goto match_done;
    }

copy_match:
    *op++ = *m_pos++; *op++ = *m_pos++;
    do *op++ = *m_pos++; while (--t > 0);

match_done:
    t = readBytes<uint8_t>(i, -2) & 3U;
    if (t == 0) goto B_3;

match_next:
    assert(t > 0 && t < 4);
    if ((op_end - op) < (t)) {
        throw std::overflow_error("Output Overrun");
    }

    *op++ = readBytes<uint8_t>(i);
    if (t > 1) { *op++ = readBytes<uint8_t>(i); if (t > 2) { *op++ = readBytes<uint8_t>(i); } }

    t = readBytes<uint8_t>(i);
    goto match;
}
