'use strict';

const SoundPlayer = require('../native-sound-player');

// Asnyc (Promise)
console.log('Playing');
SoundPlayer.play(`${__dirname}/sample.wav`)
    .then(() => console.log('Done'), e => console.error('Error:', e));
console.log('Pending');
