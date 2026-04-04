Library and Configuration Tool for the Mockingbird-OTTO firmware
================================================================

This repository contains software to configure the synthesizer firmware
developed by the Sequoia Development Group called "Mockingbird-OTTO"
which is used on Ensoniq Soundscape sound cards and third-party designs
based on that architecture. The key chips of those cards are the ODIE
interface chip, a MC68EC000 on-board processor and the OTTO sample
Wavetable Synthesizer.

There are two known major revisions of the design:
* The Ensoniq Soundscape S-1000, which uses the ODIE/OTTO combination
  for all features of the sound card and an LMC385 chip as analog
  mixer.
* The Ensoniq Soundscape S-2000, which uses a Windows Sound System
  compatible AD1848 codec for wave playback, capture and mixing, and
  uses the ODIE/OTTO combination mostly for music. The required A/D
  converter for recording and the corresponding firmware features
  have been removed. The initial firmware variant, assumed to
  still be the Mockingbird-OTTO firmware still includes wave
  playback.

The Reveal SC-600 card is a variation of the S-2000 design which
includes a TDA8421 chip for tone control. At the moment, there is no
known reason for this library to distinguish these cards, but as no
S-1000 firmware is known at the moment, the assumption that the
S-1000 firmware has the same interface is pure speculation

See the doxygen-generated documentation for more details.