////////////////////////////////////////
/// Zach Liss - University of Pittsburgh
/// Assignment 3, Compression
/// email: zll1@pitt.edu
/// date: 12/11/12
/// class: CS1550
/////////////////

/// code for lzw_encode and lzw_decode taken from:
/// http://rosettacode.org/wiki/LZW_compression

#include "header.h"

byte* lzw_encode(byte *in, int max_bits) {
	int len = _len(in), bits = 9, next_shift = 512;
	ushort code, c, nc, next_code = M_NEW;
	lzw_enc_t *d = _new(lzw_enc_t, 512);
 
	if (max_bits > 16) max_bits = 16;
	if (max_bits < 9 ) max_bits = 12;
 
	byte *out = _new(ushort, 4);
	int out_len = 0, o_bits = 0;
	uint32_t tmp = 0;
 
	inline void write_bits(ushort x) {
		tmp = (tmp << bits) | x;
		o_bits += bits;
		if (_len(out) <= out_len) _extend(out);
		while (o_bits >= 8) {
			o_bits -= 8;
			out[out_len++] = tmp >> o_bits;
			tmp &= (1 << o_bits) - 1;
		}
	}
 
	//write_bits(M_CLR);
	for (code = *(in++); --len; ) {
		c = *(in++);
		if ((nc = d[code].next[c]))
			code = nc;
		else {
			write_bits(code);
			nc = d[code].next[c] = next_code++;
			code = c;
		}
 
		/* next new code would be too long for current table */
		if (next_code == next_shift) {
			/* either reset table back to 9 bits */
			if (++bits > max_bits) {
				/* table clear marker must occur before bit reset */
				write_bits(M_CLR);
 
				bits = 9;
				next_shift = 512;
				next_code = M_NEW;
				_clear(d);
			} else	/* or extend table */
				_setsize(d, next_shift *= 2);
		}
	}
 
	write_bits(code);
	write_bits(M_EOD);
	if (tmp) write_bits(tmp);
 
	_del(d);
 
	_setsize(out, out_len);
	return out;
}

int main(int argc, char* argv[]) {
	int fd, file;
	struct stat statbuf;
	
	if( (fd = open(argv[1], O_RDONLY)) == -1) {
		fprintf(stderr, "Can't read file\n");
		return 1;
	};
	if((stat(argv[1], &statbuf)) == -1) {
		perror("Failed to get the file status");
		exit(2);
	}


	strcat(argv[1], ".Z\0");

	struct stat st;
	fstat(fd, &st);
	mode_t mode = st.st_mode;
 
	byte *in = _new(char, st.st_size);
	read(fd, in, st.st_size);
	_setsize(in, st.st_size);
	close(fd);
 
	//printf("input size:   %d\n", _len(in));
 
	byte *enc = lzw_encode(in, 9);
	//printf("encoded size: %d\n", _len(enc));

	file = open(argv[1], O_WRONLY | O_CREAT);
	write(file, enc, _len(enc));
	fchmod(file, mode);

	_del(in);
	_del(enc);
	return 0;
}
