# etc file system

A simple static file system that generates content based on predefined
config (Like suckless, but sucks).

## How to configure

Open `config.h`, append some files (currently there's no support for
directories) with it's content.

Then make and install it

```sh
make
insmod etcfs.ko
```

and after successfull installation you can use it

```sh
mount -t etcfs NULL /etc
```

## Why??

I ~~am autist~~ love programming and I want to make the most minimal usable
Linux distro with as few files as possible. This module can be build-in
directly into Linux source tree, so, in theory, it should be able to auto
mount like devtmpfs does. Minus 222 files in my linux tree.

## But what if I want to have abilty to write something into /etc?

Then you can use `overlayfs` and cry about it.
