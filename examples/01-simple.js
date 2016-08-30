'use strict';

const SoundPlayer = require('../native-sound-player.js');

console.log('Playing(1)');
SoundPlayer.playSync(`${__dirname}/sample.mp3`);

console.log('Playing(2)');
SoundPlayer.play(`${__dirname}/sample.wav`)
    .then(() => console.log('Done'), e => console.error(e));
console.log('Pending');
