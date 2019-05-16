# shittymidi
a minimal, fast, and shitty midi player that's so minimal, your synth is the bottleneck

done for fun, don't expect very serious work

# usage
drag and drop a midi file onto shittymidi.exe

if using this for any black midi stuff, i'd recommend installing omnimidi *and* using the windows multimedia wrapper patch

# resources used
[TMIDI](http://www.grandgent.com/tom/projects/tmidi/) - almost all of my code is based on this. very fast midi player that works well, but sadly the entire source code is made up of 2 files

[python-midi](https://github.com/vishnubob/python-midi) - my initial vlq parser sucked, so i ported the one from this project to c++

[The MIDI File Format](https://www.csie.ntu.edu.tw/~r92092/ref/midi/) - good reference for midi format

[Standard MIDI-File Format Spec. 1.1, updated](http://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html) - another good reference for midi format

# compatibility
i threw random midi files i had laying around on my computer at this, here's how they're currently handled

`onestop.mid` - works

`(black score) Last Brutal Sister Flandre S 110 Million Notes.mid` - plays pretty well

`Bad Apple Acer Variation.mid` - sounds perfect to me

`Septette for the Dead Princess 14.9 million.mid` - tempo might be messed up at some points

`jinjenia redzone black.mid` - sounds perfect to me

# todo (in no specific order)
- [x] fix desyncs on most midis (very very important)
- [ ] ~~use tables instead of switch case (is this actually faster?)~~ no, it's not
- [x] less buggy note counter
- [ ] better documentation
- [ ] better code overall
- [ ] piano from above-like gui (probably after everything else is done)
- [x] handle sysex (very important)
- [ ] stream from file instead of loading into memory
- [ ] command line options
