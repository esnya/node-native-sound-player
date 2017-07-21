'use strict';

const SoundPlayer = require('../native-sound-player');

console.log('Playing');
const session = SoundPlayer.play(`${__dirname}/sample.wav`);
session.then(() => console.log('Done'), e => console.error('Error:', e));
setTimeout(() => {
    console.log('Stop');
    session.stop();
}, 1000);
