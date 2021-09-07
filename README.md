# VolumeHIDtoOSC

VolumeHIDtoOSC is a simple c service that allows to use a HID device under linux to send float values for volume control. By default it is configured for a cheap USB volume controller and the master volume of the RME totalmix osc control. You can change the configuration for your needs without rebuilding it. It will be located under ``/etc/volumehidtoosc/volumehidtoosc.conf``. If something does not work as expected, you can take a look into the log file that is located under ``/var/log/volumehidtoosc/volumehidtoosc.log``.

## installing on ubuntu/debian

* download the [latest release](https://github.com/Apfelwurm/VolumeHIDtoOSC/releases)
* open your cli in the download folder and run:
```
sudo dpkg -i volumehidtoosc_*_amd64.deb
```
```
systemctl enable volumehidtoosc && systemctl start volumehidtoosc && systemctl status volumehidtoosc
```
## uninstalling on ubuntu/debian

* open your cli and run:
```
sudo dpkg -i volumehidtoosc_*_amd64.deb
```
## prerequirements for building on debian

* cmake
* build-essential
* libconfuse-dev
* liblo-dev

## building on ubuntu/debian

* install the prerequirements above
* clone this repository
```
git clone https://github.com/Apfelwurm/VolumeHIDtoOSC.git
```

To build the usable deb package, you have to go to the VolumeHIDtoOSC and run:
```
./rebuild.sh
```

To build the daemon and just run it for development purposes, you have to go to the VolumeHIDtoOSC and run:
```
./rebuild_dev.sh
```
Note: It currently has to run as root to access the hid
## configuration

| Config Key        | Usage                                                                                                         | Example           |
|-------------------|---------------------------------------------------------------------------------------------------------------|-------------------|
| VENDOR            | set the vendor id of your usb hid  (if you need help look under detect hardware)                              | 'Vendor=2341'     |
| PRODUCT           | set the product id of your usb hid  (if you need help look under detect hardware)                             | 'Product=484d'    |
| EV                | set the ev id of your usb hid  (if you need help look under detect hardware)                                  | 'EV=1f'           |
| IP                | set the ip address of your destination server                                                                 | '10.10.10.220'    |
| PORT              | set the incoming port of your destination server                                                              | '7001'            |
| OSC_PATH          | set the osc path to where the float should be sent                                                            | '/1/mastervolume' |
| VOL_PLUS          | set the vol plus command id of your usb hid  (if you need help look under detect hardware)                    | 115               |
| VOL_PLUS_TIMES    | set the amount of events received to trigger one volume change  (if you need help look under detect hardware) | 2                 |
| VOL_MINUS         | set the vol minus command id of your usb hid  (if you need help look under detect hardware)                   | 114               |
| VOL_MINUS_TIMES   | set the amount of events received to trigger one volume change  (if you need help look under detect hardware) | 2                 |
| MUTE_TOGGLE       | set the mute toggle command id of your usb hid  (if you need help look under detect hardware)                 | 113               |
| MUTE_TOGGLE_TIMES | set the amount of events received to trigger the mute toggle  (if you need help look under detect hardware)   | 2                 |
| VOL_MIN           | set the minimal volume value of the scale                                                                     | 0.0               |
| VOL_MAX           | set the maximal volume value of the scale                                                                     | 1.0               |
| VOL_STEP          | set the amount of volume that should be added or subtracted per volume change                                 | 0.002             |
| VOL_START         | set the initial volume that should be set on startup                                                          | 0.2               |

## detect hardware

To list all hid devices, run:
```
sudo cat /proc/bus/input/devices
```

Then you have to identify the device that you are using either by name or the product/ vendor id. Then you have to look for the ``Handlers`` key, there should be at least an ``eventXX`` in the value.  
If you find multiple entrys for your device, try them one after the other.

To get the output of your hid parsed, install [debughidevent](https://github.com/Apfelwurm/debughidevent) and run (replace XX with the numbers you found out in the last step):
```
sudo /usr/bin/debughidevent /dev/input/eventXX
```

now turn / press the knob and configure the ``VOL_PLUS``, ``VOL_MINUS`` and ``MUTE_TOGGLE`` to the specific keycode. If multiple of the same keycodes are received on one press / turn, set ``VOL_PLUS_TIMES``, ``VOL_MINUS_TIMES`` and ``MUTE_TOGGLE_TIMES`` to the count.

If you are sure which device is the right one, configure ``VENDOR``, ``PRODUCT`` and ``EV``.

Restart the service using:

```
systemctl restart volumehidtoosc
```

## contributions

if you want to contribute to the project or if you have problems, dont hesitate to open issues or submit pull requests.


