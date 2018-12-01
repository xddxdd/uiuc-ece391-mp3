#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

// Commands to control SB16
#define SB16_CMD_SAMPLING_RATE 0x41
#define SB16_CMD_PAUSE 0xd0
#define SB16_CMD_CONTINUE 0xd4
#define SB16_CMD_EXIT_AFTER_BLOCK 0xda
#define SB16_CMD_PLAY 0xc6

#define SB16_MODE_MONO 0x00
#define SB16_MODE_STEREO 0x20
#define SB16_MODE_SIGNED 0x10
#define SB16_MODE_UNSIGNED 0x00

#define SB16_CHUNK_SIZE 0x8000
char sb16_buf[SB16_CHUNK_SIZE];

// Visit http://soundfile.sapp.org/doc/WaveFormat/ for more information.
typedef struct {
    uint32_t chunkID;       // Should be 'RIFF' in big endian
    uint32_t chunkSize;     // File size
    uint32_t format;        // Should be 'WAVE' in big endian

    uint32_t subchunk1ID;   // Should be 'fmt ' in big endian
    uint32_t subchunk1Size; // Should be 0x10
    uint16_t audioFormat;
    uint16_t numChannels;   // 1 or 2
    uint32_t sampleRate;    // Up to 44100
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    uint32_t subchunk2ID;   // Should be 'data' in big endian
    uint32_t subchunk2Size;
} wav_header_t;

int main() {
    // Get filename
    char fname[1024];
    if(0 != ece391_getargs((uint8_t*) fname, 1024)) {
        ece391_fdputs(1, (uint8_t*) "could not read arguments\n");
	    return 3;
    }

    // Open wav file
    int32_t aux_fd, wav_fd;
    if(-1 == (wav_fd = ece391_open((uint8_t*) fname))) {
        ece391_fdputs(1, (uint8_t*) "file not found\n");
        return 2;
    }

    // Read and verify wav header
    wav_header_t wav_header;
    ece391_read(wav_fd, &wav_header, sizeof(wav_header));
    if(0x46464952 != wav_header.chunkID             // RIFF
        || 0x45564157 != wav_header.format          // WAVE
        || 0x20746d66 != wav_header.subchunk1ID     // fmt
        || 0x10 != wav_header.subchunk1Size
        || 1 != wav_header.audioFormat
        || 0 == wav_header.numChannels
        || 0x61746164 != wav_header.subchunk2ID     // data
    ) {
        ece391_fdputs(1, (uint8_t*) "invalid wav file\n");
        return 2;
    }

    // Checking if wav file is supported
    if(wav_header.numChannels > 2) {
        ece391_fdputs(1, (uint8_t*) "channel no. > 2 is not supported\n");
        return 2;
    }
    if(wav_header.sampleRate > 44100) {
        ece391_fdputs(1, (uint8_t*) "sample rate > 44100 is not supported\n");
        return 2;
    }
    if(wav_header.bitsPerSample != 8) {
        ece391_fdputs(1, (uint8_t*) "bit depth != 8 bit is not supported\n");
        return 2;
    }

    // Print out basic information
    ece391_itoa(wav_header.numChannels, (uint8_t*) fname, 10);
    ece391_fdputs(1, (uint8_t*) fname);
    ece391_fdputs(1, (uint8_t*) " ch, ");
    ece391_itoa(wav_header.sampleRate, (uint8_t*) fname, 10);
    ece391_fdputs(1, (uint8_t*) fname);
    ece391_fdputs(1, (uint8_t*) " sample rate\n");

    // Open sound card
    if(-1 == (aux_fd = ece391_open((uint8_t*) "aux"))) {
        ece391_fdputs(1, (uint8_t*) "sound card not found\n");
        return 2;
    }

	// Copy over first chunk of music
	int32_t size;
    int i;
	size = ece391_read(wav_fd, sb16_buf, SB16_CHUNK_SIZE);
	if(size <= 0) return 4;
    for(i = size; i < SB16_CHUNK_SIZE; i++) sb16_buf[i] = 0;
	if(size != ece391_write(aux_fd, sb16_buf, size)) return 5;

	// Copy over second chunk
	size = ece391_read(wav_fd, sb16_buf, SB16_CHUNK_SIZE);
	if(size < 0) return 6;
    for(i = size; i < SB16_CHUNK_SIZE; i++) sb16_buf[i] = 0;
	if(size > 0) {
		if(size != ece391_write(aux_fd, sb16_buf, size)) return 7;
	}

	// Initialize SB16, note that we have to send command manually using ioctl
	if(-1 == ece391_ioctl(aux_fd, SB16_CMD_SAMPLING_RATE)) return 8;
	if(-1 == ece391_ioctl(aux_fd, (wav_header.sampleRate >> 8) & 0xff)) return 8;
	if(-1 == ece391_ioctl(aux_fd, wav_header.sampleRate & 0xff)) return 8;
	if(-1 == ece391_ioctl(aux_fd, SB16_CMD_PLAY)) return 8;
	if(-1 == ece391_ioctl(aux_fd,
        (wav_header.numChannels == 2 ? SB16_MODE_STEREO : SB16_MODE_MONO) | SB16_MODE_UNSIGNED)) return 8;
	if(-1 == ece391_ioctl(aux_fd, (SB16_CHUNK_SIZE - 1) & 0xff)) return 8;
	if(-1 == ece391_ioctl(aux_fd, ((SB16_CHUNK_SIZE - 1) >> 8) & 0xff)) return 8;

	// Continue playing the following blocks
	while(1) {
		// Wait until one block finished
		if(-1 == ece391_read(aux_fd, (char*) 0, 0)) return 9;
		// Tell SB16 to stop temporarily after this block, to handle slow FS operations better
		if(-1 == ece391_ioctl(aux_fd, SB16_CMD_EXIT_AFTER_BLOCK)) return 10;

		// Read the next chunk of data, copy into block correspondingly
		size = ece391_read(wav_fd, sb16_buf, SB16_CHUNK_SIZE);
        for(i = size; i < SB16_CHUNK_SIZE; i++) sb16_buf[i] = 0;
		if(size < 0) return 4;	// FS error occured
		if(size == 0) break;		// Music has finished
		if(size != ece391_write(aux_fd, sb16_buf, size)) return 11;
		if(-1 == ece391_ioctl(aux_fd, SB16_CMD_CONTINUE)) return 12;
		if(size < SB16_CHUNK_SIZE) {
			// Wait until second last block finished
			if(-1 == ece391_read(aux_fd, (char*) 0, 0)) return 13;
			// The remaining data isn't sufficient for one block
			// Finish after this block
			if(-1 == ece391_ioctl(aux_fd, SB16_CMD_EXIT_AFTER_BLOCK)) return 14;
			break;
		}
	}
	if(-1 == ece391_close(wav_fd)) return 15;
	if(-1 == ece391_close(aux_fd)) return 16;

    return 0;
}
