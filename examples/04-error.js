'use strict';

const SoundPlayer = require('../native-sound-player');

console.log('Playing');
SoundPlayer.play(`${__dirname}/not-found.wav`)
    .then(() => console.log('Done'), e => console.error('Error:', e));
console.log('Pending');
