# YACER - Yet Another CoreELEC Repo

Unofficial test builds of [CoreELEC](https://coreelec.org) for one box, the
**Ugoos AM9 Pro** (S905X5), with a few features its HDMI 2.1 output makes
possible, such as 3D with shutter glasses on TVs without 3D, over VRR.

> Thank you to the CoreELEC, LibreELEC and Kodi teams: everything here stands
> on their work. CoreELEC brings Kodi to Amlogic boxes: the kernel, the video
> and display drivers, the Dolby Vision and HDR output, the settings and the
> update system. [LibreELEC](https://libreelec.tv) built the system and the
> build tree CoreELEC grew from, and [Kodi](https://kodi.tv) is the player.
> These builds are CoreELEC 22 with some patches on top.

- **[Guides](https://github.com/allolive/yacer/wiki)**: installing, and
  building it yourself.
- **[Change list](https://allolive.github.io/yacer/)**: every difference from
  CoreELEC, patch by patch.

## One box, test builds

- **Tested on the Ugoos AM9 Pro only.** The image may also start on other
  Amlogic boxes, but nothing is tested there and nothing is supported.
- **Not an official CoreELEC release**, and no warranty: install at your own
  risk. Please report problems with these builds in
  [this repository's issues](https://github.com/allolive/yacer/issues), not on
  CoreELEC's forum or GitHub: the CoreELEC team cannot support changes they did
  not make.

## Installing

Each [release](https://github.com/allolive/yacer/releases) holds an image
for a new SD card or USB stick (`-Ugoos_AM9_Pro.img.gz`) and the update file
(`.tar`). The [install guide](https://github.com/allolive/yacer/wiki/Install-and-configure)
goes through it step by step: preparing Android, writing the image, adding
`dovi.ko` for Dolby Vision, the first boot, the remote and updates.

## Releases

Every new build is published as a
[release](https://github.com/allolive/yacer/releases) and offered to the box by
CoreELEC's own updater (*Settings > CoreELEC > Updates*, channel `YACER-22`):
with *Automatic Updates* set to `auto` it installs itself, as an official
CoreELEC update would.

## For developers

This repository holds only what is added to CoreELEC: a build checks out the
CoreELEC commit named in `coreelec.pin`, lays this repository over it and
builds the result. [Building locally](https://github.com/allolive/yacer/wiki/Building-locally)
explains how.

## License

The files of this repository are GPL-2.0-or-later ([LICENSE](LICENSE)). The
patches change other projects' code and keep that code's license; each one
names its author in its header. CoreELEC's own license and copyright notes
are in [its repository](https://github.com/CoreELEC/CoreELEC).
