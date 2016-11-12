#include <sent/stddefs.h>
#include <sent/speech.h>
#include <sent/adin.h>

boolean adin_mic_standby(int sfreq, void *dummy)
{
    return TRUE;
}

boolean adin_mic_open(char *arg)
{
    return TRUE;
}

boolean adin_mic_begin(char *arg)
{
    return TRUE;
}

boolean adin_mic_end()
{
    return TRUE;
}

int adin_mic_read(SP16 *buf, int sampnum)
{
    return 0;
}

boolean adin_mic_pause()
{
    return TRUE;
}

boolean adin_mic_terminate()
{
    return TRUE;
}

boolean adin_mic_resume()
{
    return TRUE;
}

char *adin_mic_input_name()
{
    return "Microphone";
}


int adin_file_read(SP16 *buf, int sampnum)
{
    return 0;
}

char *adin_file_get_current_filename()
{
    return "Microphone";
}




boolean adin_file_begin(char *arg)
{
    return TRUE;
}

boolean adin_file_standby(int sfreq, void *dummy)
{
    return TRUE;
}



boolean adin_stdin_standby(int sfreq, void *dummy)
{
    return TRUE;
}

boolean adin_stdin_begin(char *arg)
{
    return TRUE;
}

boolean adin_file_end()
{
    return TRUE;
}

int adin_stdin_read(SP16 *buf, int sampnum)
{
    return 0;
}

char *adin_stdin_input_name()
{
    return "Stdin";
}
