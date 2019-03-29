# shittymidi
a minimal, fast, and shitty midi player that isn't fit for any use cases

done for fun, don't expect very serious work

# usage
drag and drop a midi file onto shittymidi.exe

# resources used
[TMIDI](http://www.grandgent.com/tom/projects/tmidi/) - almost all of my code is based on this. very fast midi player that works well, but sadly the entire source code is made up of 2 files

[python-midi](https://github.com/vishnubob/python-midi) - my initial vlq parser sucked, so i ported the one from this project to c++

[The MIDI File Format](https://www.csie.ntu.edu.tw/~r92092/ref/midi/) - good reference for midi format

[Standard MIDI-File Format Spec. 1.1, updated](http://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html) - another good reference for midi format

# todo (in no specific order)
- [ ] fix desyncs on most midis (very very important)
- [ ] use tables instead of switch case (is this actually faster?)
- [ ] less buggy note counter
- [ ] better documentation
- [ ] better code overall
- [ ] piano from above-like gui (probably after everything else is done)
- [ ] handle sysex (very important)
- [ ] stream from file instead of loading into memory
- [ ] command line options
