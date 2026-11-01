# Yet Another CoreELEC Repo

Test builds of [CoreELEC](https://coreelec.org) for one box, the **Ugoos AM9
Pro** (S905X5), with a few features its HDMI 2.1 output makes possible.

> Everything here stands on other people's work. CoreELEC brings Kodi to
> Amlogic boxes: the kernel, the video and display drivers, the Dolby Vision
> and HDR output, the settings and the update system.
> [LibreELEC](https://libreelec.tv) built the system and the build tree
> CoreELEC grew from, and [Kodi](https://kodi.tv) is the player. These builds
> are CoreELEC 22 with some patches on top, rebuilt whenever it moves.

**[Guides](https://github.com/allolive/yacer/wiki)** - what the builds add and
how to set it up. **[Change list](https://allolive.github.io/yacer/)** - every
difference from CoreELEC, patch by patch.

## One box, test builds

- **Tested on the Ugoos AM9 Pro only.** The image is CoreELEC's own
  `Amlogic-no.aarch64` one, so it starts on other Amlogic boxes too, but
  nothing is tested there and nothing is supported.
- **Test builds, made by one person.** Any build can break something that
  worked in the one before. Every build stays downloadable, and an earlier one
  is always there to go back to.
- **Not an official CoreELEC release**, and no warranty: install at your own
  risk. A problem with these builds belongs in
  [this repository's issues](https://github.com/allolive/yacer/issues), not with
  CoreELEC, who did not write the code that changed.

Update tars only; fresh-flash images are not published. Start from an
[official CoreELEC release](https://coreelec.org), then copy the `.tar` from
any [release](https://github.com/allolive/yacer/releases) here to the box's
`.update` share and reboot. After that, *Settings > CoreELEC > Updates* offers
the `YACER-22` channel: set *Automatic Updates* to `auto` and new builds arrive
like CoreELEC's own, or pick any build, an older one included, under
*Available Versions*.

## How the builds are made

This repository holds only what is added to CoreELEC. A build checks out the
CoreELEC commit named in **`coreelec.pin`** from
[allolive/CoreELEC](https://github.com/allolive/CoreELEC), a plain mirror of
`CoreELEC/CoreELEC`, lays this repository over it, and builds the result.
Nothing is replayed onto upstream, so an upstream rewrite cannot conflict with
it.

The pin moves only when the new upstream is known to work with these patches.
Every day the upstream sync mirrors `CoreELEC/CoreELEC` and, when its tip is
not the pinned commit, validates it: every package's patch stack applies and
the native tests pass. It then commits the new pin here. The mirror keeps each
candidate as a `coreelec-candidate/<sha>` tag while it is validated, and each
pinned commit as a `coreelec-pin/<sha>` tag, so an upstream force-push cannot
take them away. Until then every build keeps using the last CoreELEC that
worked. A change that needs a newer upstream (re-anchoring the patches, say)
moves the pin itself, in the same commit.

The workflows write to the mirror with its deploy key, stored here as the
`MIRROR_DEPLOY_KEY` secret (`scripts/yacer-mirror.sh`).

## Layout

- **`patches-yacer/<group>/<pkg>/`** — the patches to packages, grouped by what
  they are for. A `kodi-` group touches only upstream Kodi code, a `coreelec-`
  group is Amlogic-specific, and `yacer-` is for these builds alone. Hardening leads each
  family. Each group carries a `README.md`: its heading names the group and the
  text under it says what the patches are for, which is what the
  [change list](https://allolive.github.io/yacer/) shows above them. Without
  one a group is named after its directory and shown bare.

  A patch is numbered within its group — `01`, `02` — and assembly composes the
  group's number in as it copies, so `11-coreelec-av_sync/kodi/01-…` lands as
  `projects/Amlogic-ce/patches-yacer/kodi/11_01-…`. That name is what the build
  sorts on: the group's number decides where its patches sit among the other
  groups', the patch's own where it sits inside its group. Renumbering or
  renaming a group is then renaming one directory, not every patch in it.
- **`tree-patches/`** — the few upstream files that are changed, as patches. If
  upstream moves that code the patch stops applying and the build stops with it,
  rather than quietly restoring an older copy.
- **`overlay/`** — anything else added to the tree, copied in as-is: today
  the hunspell and cld2 packages the subtitle repair links into Kodi. A path that already exists upstream is refused rather than overwritten:
  replacing a whole upstream file would silently revert whatever they changed in
  it, with nothing to notice.

These patches are applied exactly the way upstream's are, and show up in the
build log as `(yacer)` so it is clear which is which.

Each patch's subject and message are what the
[change list](https://allolive.github.io/yacer/) shows, so write them for
the people installing the build. `scripts/yacer-changes.py` builds that page
locally; `.github/workflows/yacer-docs.yml` publishes it whenever a patch
changes, without committing anything or starting a build.

## Building locally

Two checkouts side by side, so this repository can be edited while the build
tree stays a build tree:

    ~/Documents/coreelec/CoreELEC        allolive/CoreELEC - the build tree
    ~/Documents/coreelec/CoreELEC-yacer  this repository - where the patches live

The build tree has to hold the pinned commit:

    git -C CoreELEC fetch origin refs/tags/coreelec-pin/$(cat CoreELEC-yacer/coreelec.pin)
    git -C CoreELEC checkout --detach $(cat CoreELEC-yacer/coreelec.pin)

Then, in the CoreELEC checkout:

    ../CoreELEC-yacer/scripts/yacer-local.sh apply         # lay this repository over the tree
    ../CoreELEC-yacer/scripts/yacer-build-local.sh        # build it
    ../CoreELEC-yacer/scripts/yacer-local.sh revert       # back to a plain mirror

`yacer-build-local.sh` runs the same `make release` CI does, with two things a
plain `make` gets wrong on a workstation: it keeps the compiler cache outside
the build tree, where `make distclean` cannot delete it, and it stops up front
if `libcrypt-dev` is absent rather than letting `heimdal:host` fail to link a
quarter of an hour in. `TARGET=image` adds the fresh-flash `.img.gz`; `CLEAN=1`
starts from a distclean.

`apply` takes the files from the `yacer` checkout as they are, so an edit can be
built without being committed first - `YACER_SOURCE=git` forces the committed
tree instead, for building exactly what CI will. It keeps a copy of what it laid
down so `revert` removes exactly that. It refuses to start from a build tree
that is not at the mirror state the tree-patches were written against, refuses a
dirty copy of anything they edit, and refuses a patch that also sits loose in the
package's own patch directory - which would otherwise be applied twice.

## Releases

Every build gets its own tag and release, carrying the update `.tar`, its
checksum, and links to the two commits it was built from. Nothing is ever
deleted: an old build stays downloadable from its own release page for as long
as the repository exists.

The `yacer-feed` release holds the catalogue the box reads, rebuilt from the
releases that are actually there. Every build is listed by default; one number in
the feed workflow caps it, and raising that cap brings older ones back, since
they were never removed.

Source archives are mirrored on the `packages-mirror` release and seeded before
the build, so a build does not depend on third-party hosts staying reachable.

## License

Original code is released under GPLv2.

## Copyright

As CoreELEC includes code from many upstream projects it includes many copyright owners. CoreELEC makes NO claim of copyright on any upstream code. Patches to upstream code have the same license as the upstream project, unless specified otherwise. For a complete copyright list please checkout the source code to examine license headers. Unless expressly stated otherwise all code submitted to the CoreELEC project (in any form) is licensed under GPLv2. You are absolutely free to retain copyright. To retain copyright simply add a copyright header to each submitted code page. If you submit code that is not your own work it is your responsibility to place a header stating the copyright.
