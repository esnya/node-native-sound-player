'use strict';

const SoundPlayer = require('../native-sound-player.js');

// Sync
console.log('Playing(1)');
SoundPlayer.playSync(`${__dirname}/sample.mp3`);

// Asnyc (Promise)
console.log('Playing(2)');
SoundPlayer.play(`${__dirname}/sample.wav`)
    .then(() => console.log('Done(2)'), e => console.error('Error:', e));
console.log('Pending(2)');

// Error
console.log('Playing(3)');
SoundPlayer.play(`${__dirname}/not-found.wav`)
    .then(() => console.log('Done(3)'), e => console.error('Error:', e));
console.log('Pending(3)');
