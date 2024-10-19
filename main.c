#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <portaudio.h>

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 256
#define THRESHOLD 0.1f  // Limiar para compressão
#define RATIO 4.0f      // Razão de compressão (4:1)
#define ATTACK 0.01f    // Velocidade de ataque
#define RELEASE 0.1f    // Velocidade de liberação
#define MAKEUP_GAIN 1.5f // Ganho de maquiagem (ajuste final de ganho)

typedef struct {
    float gain;
} CompressorData;

/* Função callback chamada pelo PortAudio para processar o áudio */
static int compressorCallback(const void *inputBuffer, void *outputBuffer,
                              unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo* timeInfo,
                              PaStreamCallbackFlags statusFlags,
                              void *userData) {
    float *in = (float*)inputBuffer;
    float *out = (float*)outputBuffer;
    CompressorData *compressorData = (CompressorData*)userData;
    unsigned int i;

    for (i = 0; i < framesPerBuffer; i++) {
        float sample = *in++;
        
        // Aplicar compressão
        float abs_sample = fabs(sample);

        if (abs_sample > THRESHOLD) {
            float overThreshold = abs_sample - THRESHOLD;
            float compressed = THRESHOLD + overThreshold / RATIO;

            // Atualiza o ganho suavemente (attack/release)
            if (compressed < abs_sample) {
                compressorData->gain -= ATTACK * (compressorData->gain - compressed);
            } else {
                compressorData->gain += RELEASE * (compressed - compressorData->gain);
            }

            sample = copysignf(compressorData->gain, sample); // Aplicar ganho
        }

        // Aplicar makeup gain (ajuste final)
        sample *= MAKEUP_GAIN;

        // Enviar para o buffer de saída
        *out++ = sample;
    }

    return 0;
}



typedef struct
{
    float left_phase;
    float right_phase;
} paTestData;

static int patestCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    /* Cast data passed through stream to our structure. */
    paTestData *data = (paTestData*)userData; 
    float *out = (float*)outputBuffer;
    unsigned int i;
    (void) inputBuffer; /* Prevent unused variable warning. */
    
    for( i=0; i<framesPerBuffer; i++ )
    {
        *out++ = data->left_phase;  /* left */
        *out++ = data->right_phase;  /* right */
        /* Generate simple sawtooth phaser that ranges between -1.0 and 1.0. */
        data->left_phase += 0.01f;
        /* When signal reaches top, drop back down. */
        if( data->left_phase >= 1.0f ) data->left_phase -= 2.0f;
        /* higher pitch so we can distinguish left and right. */
        data->right_phase += 0.03f;
        if( data->right_phase >= 1.0f ) data->right_phase -= 2.0f;
    }
    return 0;
}

int main(void) {
    PaStream *stream;
    PaDeviceInfo *devInfo;
    PaError err;
    CompressorData compressorData = {1.0f}; // Iniciar o ganho no máximo

    /* Inicializar PortAudio */
    err = Pa_Initialize();
    if (err != paNoError) {
        printf("Erro PortAudio: %s\n", Pa_GetErrorText(err));
        return 1;
    }


    /* Select device */
    int numDev = Pa_GetDefaultInputDevice();

    /* Abrir o stream de captura e reprodução */
    // err = Pa_OpenDefaultStream(&stream,
    //                            1,          /* 1 canal de entrada (mono) */
    //                            1,          /* 1 canal de saída (mono) */
    //                            paFloat32,  /* Formato de amostra de 32 bits float */
    //                            SAMPLE_RATE,
    //                            FRAMES_PER_BUFFER,
    //                            patestCallback,
    //                            &compressorData);  /* Dados do compressor */

    err = Pa_OpenStream(
        &stream,
        &(PaStreamParameters){Pa_GetDefaultInputDevice(), paFloat32, 1, Pa_GetDeviceInfo(Pa_GetDefaultInputDevice())->defaultLowInputLatency, NULL},
        &(PaStreamParameters){Pa_GetDefaultOutputDevice(), paFloat32, 1, Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice())->defaultLowOutputLatency, NULL},
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff,
        compressorCallback,
        &compressorData
    );


    if (err != paNoError) {
        printf("Erro PortAudio: %s\n", Pa_GetErrorText(err));
        Pa_Terminate();
        return 1;
    }

    /* Iniciar o stream */
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        printf("Erro PortAudio: %s\n", Pa_GetErrorText(err));
        Pa_Terminate();
        return 1;
    }

    printf("Rodando stream de áudio... Pressione Enter para parar.\n");
    getchar(); /* Espera até que o usuário pressione Enter */

    /* Parar o stream */
    err = Pa_StopStream(stream);
    if (err != paNoError) {
        printf("Erro PortAudio: %s\n", Pa_GetErrorText(err));
    }

    /* Fechar o stream */
    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        printf("Erro PortAudio: %s\n", Pa_GetErrorText(err));
    }

    /* Finalizar PortAudio */
    Pa_Terminate();

    printf("Execução finalizada.\n");
    return 0;
}
