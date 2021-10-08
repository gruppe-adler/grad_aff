#include "../include/grad_aff/a3lzss.h"
#include <cstddef>
#include <cstring>

#include "grad_aff/StreamUtil.h"

#include <iostream>
#include <vector>

using namespace grad_aff;

std::vector<uint8_t> LzssDecompressUnknownLength(std::istream& is) {

	std::vector<uint8_t> out = {};
	unsigned unsigned long outlen = LLONG_MAX;
	char Flag;
	char bits;
	long Rpos;
	char rlen;
	//int Fl = 0;
	unsigned long CalculatedChecksum = 0;
	unsigned long ReadChecksum;
	//int pi = 0;
	char data;
	uint8_t* to, * from;

	while (outlen > 0)
	{
		Flag = readBytes<uint8_t>(is);
		//Flag = in[pi++];
		for (bits = 0; bits < 8; bits++, Flag >>= 1)// up to 8 bytes of data or 8 pointers or
		{
			if (Flag & 0x01) // raw data
			{
				data = readBytes<uint8_t>(is);
				//data = in[pi++];
				CalculatedChecksum += data;
				out.push_back(data);
				//OutBuf[Fl++] = data;
				if (!--outlen) goto finish;
				continue;
			}
			Rpos = readBytes<uint8_t>(is);
			//Rpos = in[pi++];
			rlen = (peekBytes<uint8_t>(is) & 0x0F) + 3;
			//rlen = (in[pi] & 0x0F) + 3;
			Rpos += ((readBytes<uint8_t>(is) & 0xF0) << 4);
			//Rpos += ((in[pi++] & 0xF0) << 4);
			//while (Rpos > Fl) // special case space fill
			while(Rpos > out.size())
			{
				CalculatedChecksum += 0x20;
				out.push_back(0x20);
				//OutBuf[Fl++] = 0x20;
				if (!--outlen) goto finish;
				if (!--rlen) goto stop;
			}
			Rpos = out.size() - Rpos;
			//Rpos = Fl - Rpos;
			from = &out[Rpos];
			//from = &OutBuf[Rpos];
			to = &out[out.size() - 1];
			//to = &OutBuf[Fl];
			//Fl += rlen;
			while (rlen--)
			{
				data = *from++;
				CalculatedChecksum += data;
				*to++ = data;
				if (!--outlen) goto finish;
			}
		stop:;
		}
	}
	goto ok;
finish:
	if (Flag & 0xFE) return {}; // EXCESS_1BITS;
ok:
	auto checksum = readBytes<unsigned long>(is);
	//std::memcpy(&ReadChecksum, checksum, sizeof(ReadChecksum))
//	std::memcpy(&ReadChecksum, &in[pi], sizeof(ReadChecksum));
	if (ReadChecksum == CalculatedChecksum) return out;
	//if (ReadChecksum == CalculatedChecksum) return pi + 4
	return {};
}

int ExpandUnknownInputLength(const char* in, char* OutBuf, long outlen)
{
	char Flag;
	char bits;
	long Rpos;
	char rlen;
	int Fl = 0;
	unsigned long CalculatedChecksum = 0;
	unsigned long ReadChecksum;
	int pi = 0;
	char data;
	char* to, * from;

	while (outlen > 0)
	{
		Flag = in[pi++];
		for (bits = 0; bits < 8; bits++, Flag >>= 1)// up to 8 bytes of data or 8 pointers or
		{
			if (Flag & 0x01) // raw data
			{
				data = in[pi++];
				CalculatedChecksum += data;
				OutBuf[Fl++] = data;
				if (!--outlen) goto finish;
				continue;
			}
			Rpos = in[pi++];
			rlen = (in[pi] & 0x0F) + 3;
			Rpos += ((in[pi++] & 0xF0) << 4);
			while (Rpos > Fl) // special case space fill
			{
				CalculatedChecksum += 0x20;
				OutBuf[Fl++] = 0x20;
				if (!--outlen) goto finish;
				if (!--rlen) goto stop;
			}
			Rpos = Fl - Rpos;
			from = &OutBuf[Rpos];
			to = &OutBuf[Fl];
			Fl += rlen;
			while (rlen--)
			{
				data = *from++;
				CalculatedChecksum += data;
				*to++ = data;
				if (!--outlen) goto finish;
			}
		stop:;
		}
	}
	goto ok;
finish:
	if (Flag & 0xFE) return -1; // EXCESS_1BITS;
ok:
	std::memcpy(&ReadChecksum, &in[pi], sizeof(ReadChecksum));
	if (ReadChecksum == CalculatedChecksum) return pi + 4;
	return -1;
}