const SoundPlayer = require('./native-sound-player');

process.on('message', message => {
    SoundPlayer.playSync.apply(SoundPlayer, message.args);
    process.send(null);
    process.exit(0);
});
