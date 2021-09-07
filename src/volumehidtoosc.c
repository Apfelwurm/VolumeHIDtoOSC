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
#include <confuse.h>
#include "lo/lo.h"

cfg_opt_t opts[] =
{
    CFG_STR("VENDOR", "Vendor=2341", CFGF_NONE),
    CFG_STR("PRODUCT", "Product=484d", CFGF_NONE),
    CFG_STR("EV", "EV=1f", CFGF_NONE),
    CFG_STR("IP", "10.10.10.220", CFGF_NONE),
    CFG_STR("PORT", "7001", CFGF_NONE),
    CFG_STR("OSC_PATH", "/1/mastervolume", CFGF_NONE),
    CFG_INT("VOL_PLUS",115,CFGF_NONE),
    CFG_INT("VOL_PLUS_TIMES",2,CFGF_NONE),
    CFG_INT("VOL_MINUS",114,CFGF_NONE),
    CFG_INT("VOL_MINUS_TIMES",2,CFGF_NONE),
    CFG_INT("MUTE_TOGGLE",113,CFGF_NONE),
    CFG_INT("MUTE_TOGGLE_TIMES",2,CFGF_NONE),
    CFG_FLOAT("VOL_MIN",0.0,CFGF_NONE),
    CFG_FLOAT("VOL_MAX",1.0,CFGF_NONE),
    CFG_FLOAT("VOL_STEP",0.002,CFGF_NONE),
    CFG_FLOAT("VOL_START",0.3,CFGF_NONE),
    CFG_END()
};
cfg_t *cfg;

lo_address t;



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


        if (strstr(buffer, cfg_getstr(cfg, "PRODUCT"))) {
            productmatch = true;	        
        }
        if (strstr(buffer, cfg_getstr(cfg, "VENDOR"))) {
            vendormatch = true;

        }
        if (strstr(buffer, cfg_getstr(cfg, "EV"))) {
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

int sendosc(float currvol)
{


    if (lo_send(t, cfg_getstr(cfg, "OSC_PATH"), "ff", currvol) == -1) {
        printf("OSC error %d: %s\n", lo_address_errno(t), lo_address_errstr(t));
    }
    return 0;
}


int main(void)
{
    int fd;
    struct input_event evt;
    int volpluscount = 0;
    int volminuscount = 0;
    int mutetogglecount = 0;
    float sentvolume = 0.0;
    float volume = 0.0;
    bool mutetrigger = false;
    bool unmutetrigger = false;
    bool mute = false;
    char *evdev;

    cfg = cfg_init(opts, CFGF_NONE);
    if(cfg_parse(cfg, "/etc/volumehidtoosc/volumehidtoosc.conf") == CFG_PARSE_ERROR)
    {
        printf("Config error");
        exit(1);
    }

    
    

    t = lo_address_new(cfg_getstr(cfg, "IP"), cfg_getstr(cfg, "PORT"));
    
    volume = cfg_getfloat(cfg, "VOL_START");
    sendosc(volume);
    sentvolume = volume;

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


         if (!mute)
        {
            if (evt.code == cfg_getint(cfg, "VOL_PLUS"))
            {
                volpluscount++;
            }
            if (evt.code == cfg_getint(cfg, "VOL_MINUS"))
            {
                volminuscount++;
            }
        }
        if (evt.code == cfg_getint(cfg, "MUTE_TOGGLE"))
        {
            mutetogglecount++;  
        }


        if (volpluscount == cfg_getint(cfg, "VOL_PLUS_TIMES"))
        {
            volpluscount=0;
            if ((volume + cfg_getfloat(cfg, "VOL_STEP") ) <= cfg_getfloat(cfg, "VOL_MAX"))
            {
            volume = volume + cfg_getfloat(cfg, "VOL_STEP");
            }
            else
            {
                volume = cfg_getfloat(cfg, "VOL_MAX");
            }

        }        
        if (volminuscount == cfg_getint(cfg, "VOL_MINUS_TIMES"))
        {
            volminuscount=0;
            if ((volume - cfg_getfloat(cfg, "VOL_STEP") ) >= cfg_getfloat(cfg, "VOL_MIN"))
            {
                volume = volume - cfg_getfloat(cfg, "VOL_STEP");
            }
            else
            {
                volume = cfg_getfloat(cfg, "VOL_MIN");
            }
        }        
        if (mutetogglecount == cfg_getint(cfg, "MUTE_TOGGLE_TIMES"))
        {
            mutetogglecount=0;
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
                printf("sendvol  %f\n", volume);
                sendosc(volume);
                sentvolume = volume;
            }
        }

        if(mutetrigger)
        {
            printf("sendvol  %f\n", cfg_getfloat(cfg, "VOL_MIN"));
            sendosc(cfg_getfloat(cfg, "VOL_MIN"));
            mutetrigger = false;
            mute = true;
        }
        if(unmutetrigger)
        {
            printf("sendvol  %f\n", volume);
            sendosc(volume);
            unmutetrigger = false;
            mute = false;
        }

    }


}
