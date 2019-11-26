#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sndfile.h>

//Compile this code using:
//gcc 9.1.c -o 9.1 -lsndfile

#define kSampleRate 44100
#define kSec 1
#define kNumFrames (kSampleRate * kSec)
#define kNumChannels 2
#define kFormat (SF_FORMAT_WAV | SF_FORMAT_PCM_24)
#define kFileName "TriangleWave.wav"

void plotAndPlay();

int main(void)
{
    SNDFILE *sndFile;
    SF_INFO sfInfo;
    double *buffer;

    //Sound parameters
    double amplitude = 0.25;
    double frequency = 250.0;

    //Create 2 channels worth buffer for holding audio data
    buffer = calloc(kNumChannels * kNumFrames, sizeof(double));
    if (!buffer)
    {
        printf("Error: malloc failed.\n");
        return 1;
    };

    //Initialize SF_INFO with 0s (memset is in string.h library)
    memset(&sfInfo, 0, sizeof(SF_INFO));

    //Set the format for the sound file to be saved
    sfInfo.samplerate = kSampleRate;
    sfInfo.frames = kNumFrames;
    sfInfo.channels = kNumChannels;
    sfInfo.format = kFormat;

    //Open the original sound file as read mode
    sndFile = sf_open(kFileName, SFM_WRITE, &sfInfo);
    if (!sndFile)
    {
        printf("Error : could not open file : %s\n", kFileName);
        puts(sf_strerror(NULL));
        return 1;
    }

    //Compute Triangle Wave (interleave channels)
    for (int n = 0; n < kNumFrames; n++)
    {
        // n / kSampleRate current frame number divided by sampling rate, how far through the loop are we
        for (int i = 1; i <= 100; i += 2) //Up to 15th harmonics
        {
            double sample = (amplitude / (i * i)) * sin(2.0 * M_PI * frequency * i * n / kSampleRate); 
            sample *= pow(-1.0, (i - 1.0) / 2.0); // flipping the phase and going to the right harmonic (i - 1 / 2)
            for (int c = 0; c < kNumChannels; c++)
            {
                buffer[kNumChannels * n + c] += sample;
            }
        }
    }

    //Write out the result
    sf_count_t count = sf_write_double(sndFile, buffer, sfInfo.channels * kNumFrames);

    //Make sure writing the file succeeeded
    if (count != sfInfo.channels * kNumFrames)
    {
        puts(sf_strerror(sndFile));
    }

    //Close sound file and buffer
    sf_close(sndFile);
    free(buffer);

    plotAndPlay(kFileName);

    return 0;
}

void plotAndPlay(char *fileName)
{
    //Save .dat file to plot the waveform using gnuplot
    //Install sox for this to work:
    //brew install sox
    char command[100];
    sprintf(command, "sox \"%s\" -r 8000 plot.dat", fileName);
    system(command);

    //Plot the plot.dat file
    system("gnuplot --persist -e \"plot 'plot.dat' every ::1 using 1:2 with lines\"");

    //Play the audio file for auditory feedback
    sprintf(command, "afplay \"%s\" 2> /dev/null", fileName);
    system(command);
}