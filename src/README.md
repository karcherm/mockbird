The documentation covers these topics:

Command-line setup tool
=======================
Invoke `mockbird` for these functions:

* Get command line help using `mockbird help`
* List the synthsizer configuration by `mockbird dump`.
  As of version 0.1, running the tool without any parameter
  performs the same operation.
* List the definition of certain melodic programs by
  `mockbird dumpprg <n>` or `mockbird dumpprg <m>-<n>`.
  The syntax used for specifying the up to four layers
  of a program is described in \ref Soundscape::InstrumentLayer
* Change the definition of a single melodic program by
  using `mockbird loadprg <n>`. The program definition may
  be on the command line or provided on stdin.
* Play a test tune to try modified programs by invoking
  `mockbird play`, optionally followed by a program number.
  If no program number is given, program 0 will be used.
* Reset the synthesizer using `mockbird reset`.

Note that `mockbird` uses 0-based program numbers as of version 0.1.

Library use
===========
Construct a \ref Soundscape::Card object to interface to the
synthesizer. The documentation of that object contains further
details.

Synthesizer architecture
========================

Waveforms
---------
The wavetable synthesizer uses a set of monophonic waveforms, which
may be encoded as 16-bit linear PCM, 12-bit linear PCM or 8-bit
linear or uLaw PCM. These waveforms may be looped.

The same wave data may be looped in different ways. In this case,
one waveform definition includes the wave data, and other waveforms
define different start, end, loop start and loop end positions
referring back to the "primary waveform". These referring waveforms
are called "alias waveforms".

Furthermore, there may be a map that dispatches to different
waveforms depending on the key, which may be used to provide a kind
of "MIP-mapping" for samples, or to provide different recordings
for different key ranges. These dispatching pseudo-waveforms are
called "wavemaps".

In higher layers, there is no distinction whether a waveform is
a primary waveform, an alias waveform or a wavemap.

Patches
-------
The parameters how to play a wave are specified in patches. Those
patches include definitions of the playback rate, the volume,
filter coefficients, two low-frequency oscillators and two
envelope generators. Patches are still monophonic.

Layers
------
The top layer is melodic instruments (which are addressed as MIDI
programs), which can refer to up to four patches, and drum
instruments, which refer to a single patch.

Implementation status
=====================
At version 0.1 these features are implemented
* Sending MIDI data to the synthesizer
* Reassigning patches to melodic instruments

While these features are still missing
* Reassigning patches to drum instruments
* Reprogramming patches
* Redefining waveforms
