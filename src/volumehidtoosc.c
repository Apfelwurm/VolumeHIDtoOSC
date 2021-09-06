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
#include <rtosc/rtosc.h>

cfg_opt_t opts[] =
{
    CFG_STR("VENDOR", "Vendor=2341", CFGF_NONE),
    CFG_STR("PRODUCT", "Product=484d", CFGF_NONE),
    CFG_STR("EV", "EV=1f", CFGF_NONE),
    CFG_INT("VOL_PLUS",115,CFGF_NONE),
    CFG_INT("VOL_PLUS_TIMES",2,CFGF_NONE),
    CFG_INT("VOL_MINUS",114,CFGF_NONE),
    CFG_INT("VOL_MINUS_TIMES",2,CFGF_NONE),
    CFG_INT("MUTE_TOGGLE",113,CFGF_NONE),
    CFG_INT("MUTE_TOGGLE_TIMES",2,CFGF_NONE),
    CFG_INT("VOL_MIN",0,CFGF_NONE),
    CFG_INT("VOL_MAX",100,CFGF_NONE),
    CFG_INT("VOL_STEP",1,CFGF_NONE),
    CFG_INT("VOL_START",30,CFGF_NONE),
    CFG_END()
};
cfg_t *cfg;


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


int main(void)
{
    int fd;
    struct input_event evt;
    int volpluscount = 0;
    int volminuscount = 0;
    int mutetogglecount = 0;
    int sentvolume = 0;
    int volume = 0;
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

    sleep(3);
    
    volume = cfg_getint(cfg, "VOL_START");

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
            if ((volume + cfg_getint(cfg, "VOL_STEP") ) <= cfg_getint(cfg, "VOL_MAX"))
            {
            volume = volume + cfg_getint(cfg, "VOL_STEP");
            }

        }        
        if (volminuscount == cfg_getint(cfg, "VOL_MINUS_TIMES"))
        {
            volminuscount=0;
            if ((volume - cfg_getint(cfg, "VOL_STEP") ) >= cfg_getint(cfg, "VOL_MIN"))
            {
                volume = volume - cfg_getint(cfg, "VOL_STEP");
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
                printf("sendvol  %d\n", volume);
                sentvolume = volume;
            }
        }

        if(mutetrigger)
        {
            printf("sendvol  %d\n", cfg_getint(cfg, "VOL_MIN"));
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
