/*
 *
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 * Contributor(s): Apfelwurm <volzit.de>
 *
 */



#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define EVDEV "/dev/input/event14"
#define VENDOR "Vendor=2341"
#define PRODUCT "Product=484d"
#define EV "EV=1f"
#define VOL_PLUS 115
#define VOL_PLUS_TIMES 4
#define VOL_MINUS 114
#define VOL_MINUS_TIMES 4
#define MUTE_TOGGLE 113
#define MUTE_TOGGLE_TIMES 2
#define VOL_MIN 0
#define VOL_MAX 100
#define VOL_STEP 2
#define VOL_START 30


char *extract_keyboard_eventname()
{
    FILE *fp = NULL;
    char buffer[1024];
    char *eventname = NULL;
    bool vendormatch = false;
    bool productmatch = false;
    bool evmatch = false;
    bool match = false;

    fp = fopen("/proc/bus/input/devices", "r");
    if (!fp) {
        int err = errno;
        fprintf(stderr, "Unable to open file. %s\n", strerror(err));
        return NULL;
    }
    memset(buffer, 0, sizeof(buffer));
    while (fgets(buffer, sizeof(buffer), fp)) {
        char *ptr = NULL;
        if ((ptr = strstr(buffer, "Handlers="))) {
            ptr += strlen("Handlers=");
            ptr = strstr(ptr, "event");
            if (ptr) {
                char *ptr2 = strchr(ptr, ' ');
                if (ptr2)
                    *ptr2 = '\0';
                eventname = strdup(ptr);
                if (!eventname) {
                    fprintf(stderr, "Out of memory.\n");
                    break;
                }
            }
        }


        if (strstr(buffer, PRODUCT)) {
            productmatch = true;
        }
        if (strstr(buffer, VENDOR)) {
            vendormatch = true;
        }
        if (strstr(buffer, EV)) {
            evmatch = true;
        }

        if (evmatch && productmatch && vendormatch)
        {
            match=true;
            break;
        }

        if (strstr(buffer, "MSC=")) {
            evmatch = false;
            vendormatch = false;
            productmatch = false;
        }

    }
    fclose(fp);
    if (match)
    {
        return eventname;
    }
    else
    {
        return "not found";
    }
}


int main(void)
{
    int fd;
    struct input_event evt;
    int volpluscount = 0;
    int volminuscount = 0;
    int mutetogglecount = 0;
    int sentvolume = 0;
    int volume = VOL_START;
    bool mutetrigger = false;
    bool unmutetrigger = false;
    bool mute = false;
    char *evdev;

    sleep(3);

    if (extract_keyboard_eventname() == "not found")
    {
        printf("Controller not found");
        exit(1);
    }
    else
    {
        fprintf(stderr, "selected /dev/input/%s\n", extract_keyboard_eventname());
        asprintf(&evdev, "%s%s", "/dev/input/", extract_keyboard_eventname());
    }

    fd = open(evdev, O_RDWR);
    ioctl(fd, EVIOCGRAB, 1);


    while(read(fd, &evt, sizeof(struct input_event)) > 0) {
        // printf("\n\ncaptured\n");

        // printf("code = %d\n", evt.code);
        // printf("time = %d\n", evt.time);
        // printf("val = %d\n", evt.value);
        // printf("type = %d\n", evt.type);

         if (!mute)
        {
            if (evt.code == VOL_PLUS)
            {
                volpluscount++;
            }
            if (evt.code == VOL_MINUS)
            {
                volminuscount++;
            }
        }
        if (evt.code == MUTE_TOGGLE)
        {
            mutetogglecount++;  
        }


        if (volpluscount == VOL_PLUS_TIMES)
        {
            volpluscount=0;
            // printf("LAUDER\n");
            if ((volume + VOL_STEP ) <= VOL_MAX)
            {
            volume = volume + VOL_STEP;
            }

        }        
        if (volminuscount == VOL_MINUS_TIMES)
        {
            volminuscount=0;
            // printf("LEISER\n");
            if ((volume - VOL_STEP ) >= VOL_MIN)
            {
                volume = volume - VOL_STEP;
            }
        }        
        if (mutetogglecount == MUTE_TOGGLE_TIMES)
        {
            mutetogglecount=0;
            // printf("TOGGLEMUTE\n");
            if (mute)
            {
                unmutetrigger = true;
                mutetrigger = false;
            }
            else
            {
                unmutetrigger = false;
                mutetrigger = true;
            }
        }

        if (!mute)
        {
            if (volume != sentvolume)
            {
                printf("sendvol  %d\n", volume);
                sentvolume = volume;
            }
        }

        if(mutetrigger)
        {
            printf("sendvol  %d\n", VOL_MIN);
            mutetrigger = false;
            mute = true;
        }
        if(unmutetrigger)
        {
            printf("sendvol  %d\n", volume);
            unmutetrigger = false;
            mute = false;
        }

    }


}
