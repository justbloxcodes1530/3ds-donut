#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <3ds.h>

#define AUDIO_SAMPLERATE 32000
#define AUDIO_CHANNELS NDSP_CHANNELS_MONO
#define AUDIO_BUFFER_SIZE 0x10000 // 64KB

ndspWaveBuf waveBuf[2];
short* audio_data = NULL;
u32 audio_size = 0;
bool audio_enabled = false;

int load_audio(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        return 0; // File not found
    }

    fseek(f, 0, SEEK_END);
    audio_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    audio_data = (short*)linearAlloc(audio_size);
    if (!audio_data) {
        fclose(f);
        return 0; // Memory allocation failed
    }

    fread(audio_data, 1, audio_size, f);
    fclose(f);
    return 1;
}

void audio_init() {
    ndspInit();
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    ndspChnReset(0);

    ndspChnSetInterp(0, NDSP_INTERP_POLYPHASE);
    ndspChnSetRate(0, AUDIO_SAMPLERATE);
    ndspChnSetFormat(0, NDSP_FORMAT_MONO_PCM16);

    memset(waveBuf, 0, sizeof(waveBuf));

    waveBuf[0].data_vaddr = audio_data;
    waveBuf[0].nsamples = audio_size / 4;
    waveBuf[0].looping = true;

    waveBuf[1].data_vaddr = audio_data + audio_size / 4;
    waveBuf[1].nsamples = audio_size / 4;
    waveBuf[1].looping = true;

    DSP_FlushDataCache(audio_data, audio_size);
    ndspChnWaveBufAdd(0, &waveBuf[0]);
    ndspChnWaveBufAdd(0, &waveBuf[1]);
}

void audio_exit() {
    ndspExit();
    if (audio_data) {
        linearFree(audio_data);
        audio_data = NULL;
    }
}

int main(int argc, char **argv)
{
    aptInit();
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);
    hidInit();

    // Try to load music
    if (load_audio("donut.raw")) {
        audio_init();
        audio_enabled = true;
    }

    float A = 0, B = 0;
    float i, j;
    int k;

    while (aptMainLoop())
    {

        int width = 40;
        int height = 20;
        int bufferSize = width * height;

        char output[800];
        float zbuffer[800];
        memset(output, ' ', bufferSize);
        memset(zbuffer, 0, sizeof(zbuffer));

        for (j = 0; j < 6.28; j += 0.07) {
            for (i = 0; i < 6.28; i += 0.02) {
                float c = sin(i), d = cos(j), e = sin(A), f = sin(j), g = cos(A);
                float h = d + 2, D = 1 / (c * h * e + f * g + 5);
                float l = cos(i), m = cos(B), n = sin(B);
                float t = c * h * g - f * e;
                int x = (int)(width / 2 + (width / 2 - 1) * D * (l * h * m - t * n));
                int y = (int)(height / 2 + (height / 2 - 1) * D * (l * h * n + t * m));
                int o = x + width * y;
                int N = (int)(8 * ((f * e - c * d * g) * m - c * d * e - f * g - l * d * n));
                if (height > y && y >= 0 && x >= 0 && width > x && D > zbuffer[o]) {
                    zbuffer[o] = D;
                    output[o] = ".,-~:;=!*#$@"[N > 0 ? N : 0];
                }
            }
        }

        printf("\x1b[2J"); // Clear screen
        for (k = 0; k < bufferSize; k++) {
            putchar(k % width ? output[k] : '\n');
        }
	printf("\n\n\n");
	printf("Created by JustBlox1530\n");
	printf("Press START to exit!");

        A += 0.04;
        B += 0.08;

        gfxFlushBuffers();
        gfxSwapBuffers();

        hidScanInput();
        u32 button = hidKeysDown();
        if (button & KEY_START) break;
    }

    if (audio_enabled) {
        audio_exit();
    }

    aptExit();
    gfxExit();
    hidExit();
    return 0;
}
