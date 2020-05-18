# cue_convert

cue_convert is a utility to help collectors with two main tasks:
1. compress a representative set of cue+bin files for use on mobile emulators where
   space might be limited.
2. batch convert redbook audio from cue+bin files.
   
Collectors often back up the contents of CD-based games to a hard drive, allowing them
to preserve their original media under more controlled conditions.  Many emulation 
packages allow users to load from bin+cue files, allowing the collector to enjoy their
collection without risk of damage.

While PC storage has grown to the point where the media size of most collections
will not be an issue, the same cannot be said for mobile devices.  Android devices
are powerful enough to run a variety of emulators, notably the multicore RetroArch,
but the built-in storage of most devices will not be able to handle the full media
size.  Even with external storage, the dream of large, pristine, portable collections
which can be enjoyed anywhere is still some time away.

In the meantime, collectors may elect either to compress the redbook audio of their
collection into something more manageable, or only copy key parts of their collection
to their mobile storage.  cue_convert combines the two into one utility, making use
of the OSS ogg vorbis container and codec, which is supported by many cores in
RetroArch.  Additionally, it can be pointed at a well-organized collection, and the
same source directory hierarchy will be built at a target location, with filters
being applied to weed out titles the collector wishes to skip including in their
mobile collection.

Thus, as a result of the copy and compression progress, vorbis files for all redbook
audio in the mobile collection will be made available, which can be imported manually
into a portable audio library so that collectors may enjoy their favorites audio
tracks on the go.
