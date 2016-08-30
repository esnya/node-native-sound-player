const SoundPlayer = require('./native-sound-player');

process.on('message', message => {
    try {
        SoundPlayer.playSync.apply(SoundPlayer, message.args);
        process.send(null);
    } catch (e) {
        process.send(e);
    }
    process.exit(0);
});
