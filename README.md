# Survive Extreme

This is a mod

## Building

**IF YOU ARE ON LINUX** just run `bash build_linux.sh`

Make a mod folder in your hlaf life directory

```sh
cd [STEAMAPPS]/common/Half-Life
mkdir survex
```

Copy over everything in `assets` as well as `network/delta.lst` from here to the directory. Then go into this repository into the directory with the Makefile for your system and run `make` then copy the output files to their respective folders

survex should look like this

```
survex
├── dlls
│   └── hl.so
├── cl_dlls
│   └── client.so
├── delta.lst
├── liblist.gam
├── sound
│   └── ...
```

I ASSMUE YOU KNOW WHAT YOUr"E DOING ([read this](https://twhl.info/wiki/page/Half-Life_Programming_-_Getting_Started) if you dont)