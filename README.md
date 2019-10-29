## This repo is archived and abandoned 
Neoton was a graduation project and isn't supported anymore.  
Portions of this project are reborn in [WaveRadio](https://github.com/WaveRadio) broadcasting platform.

# What is Neoton?
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fneoton%2Fneoton-daemon.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2Fneoton%2Fneoton-daemon?ref=badge_shield)

Neoton is a free and open source centralized network-based public broadcasting system. It can be used in malls, schools, universities and other places where public audio playback is needed. Neoton consists of three parts: Daemon, Endpoint and Control Panel. Now you are looking at the Daemon.

# What is Neoton Daemon?
Neoton Daemon is the server software that manages Endpoints: controls their state, sends command, sets up and controls audio stream generator processes. Endpoints connect to Daemon and pull settings from it before starting to play the stream.

# How to set it up?
You need to build Neoton from source. Binary packages will be published later. To build Neoton Daemon you will need Qt version 5.6.0 or newer. Download Daemon source, open its directory and just run `qmake && make`. That's it. You also might want to copy the `neotond` binary into directory like `/usr/local/bin` and set up a `systemd` service entry. It's on your own.

# How to configure it?
Neoton Daemon uses INI-like configuration file. To pass the configuration file to the daemon, use `-c` parameter, for example: `/usr/local/bin/neotond -c /etc/neotond.conf`. The config file is simple, just take a look at the example below.
```
[server]
; Log facility. Use stdout to print log to your console
log_file=/var/log/neoton/daemon.log
; Verbosity: 0 - none, 1 - error, 2 - warning, 3 - info, 4 - debug
log_level=4
; Server port, 1337 by default
server_port=1337

; Neoton uses PostgreSQL database to save its state. 
; You must specify database settings. 
[database]
server=127.0.0.1
database=neoton
username=neoton
password=0hW0w1t5N30tOn!

; Daemon uses Liquidsoap to build multimedia streams.
; Here you have to tell the daemon where to find all neccessary artifacts
[environment]
; The path to Liquidsoap binary executable
liquidsoap_path=/usr/bin/liquidsoap
; Config dir where Control Panel saves the .liq scripts
liquidsoap_config_dir=/opt/neoton/runtime/liquidsoap
; Restart Liquidsoap if it exits?
respawn=false
; Restart Liquidsoap only on "bad" exits like crash?
respawn_on_crash=false
```


## License
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fneoton%2Fneoton-daemon.svg?type=large)](https://app.fossa.io/projects/git%2Bgithub.com%2Fneoton%2Fneoton-daemon?ref=badge_large)