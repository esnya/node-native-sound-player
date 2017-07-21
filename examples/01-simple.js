'use strict';

const SoundPlayer = require('../native-sound-player.js');

// Sync
console.log('Playing');
SoundPlayer.play(`${__dirname}/sample.mp3`)
    .then(() => console.log('Done'))
    .catch(e => console.log('Error', e));
console.log('Pending');
