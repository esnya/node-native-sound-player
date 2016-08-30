'use strict';

const SoundPlayer = require('../native-sound-player.js');

const devices = SoundPlayer.getDevices();

devices.forEach(device => {
    console.log('id:', device.id);
    console.log('name:', device.name);

    SoundPlayer.playSync(`${__dirname}/sample.mp3`, {
        output: {
            id: device.id,
        },
    });
});
